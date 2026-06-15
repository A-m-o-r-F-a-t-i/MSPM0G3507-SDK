/*
 * Copyright (c) 2022, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 *
 * *  Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * *  Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE,
 * EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
/*
 * DMAMSPM0.c - MSPM0 平台 DMA 驱动实现
 * 提供 DMA 初始化、通道打开/关闭、传输配置、中断管理等功能。
 */
/* clang-format off */
#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include <ti/devices/DeviceFamily.h>

#include <ti/driverlib/dl_dma.h>
#include <ti/drivers/dma/DMAMSPM0.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

/* 未配置应用实例时提供空弱默认项，避免不完整数组触发 GCC 诊断。 */
__WEAK const DMAMSPM0_Cfg DMAMSPM0_Config[DMAMSPM0_DEFAULT_CONFIG_COUNT] = {0};

/*
 * Lock for exclusive access to hwiFxnArgs, hwiFxnHandlers & hwiHandles arrays
 * during DMAMSPM0_open() & DMAMSPM0_close()
 */
static SemaphoreP_Handle interruptSetupLock = NULL;

/* Bit-mask of DMA channels currently being used */
static uint32_t channelsInUse = 0;

/* Bit-mask of DMA channels for transfer is currently setup or not */
static uint32_t chTransferSet = 0;

static bool dmaInitialized = false;

static uint32_t dmaIntRefCount = 0;

/* The Hwi handle for DMA interrupt */
static HwiP_Handle hwiHandle;

/* Array of ISR pointers called when a channel causes an interrupt on DMA_INT0 */
static void (*hwiFxnHandlers[NUM_DMA_CHANNELS])(uintptr_t);

/* Array of ISR arguments */
static uintptr_t hwiFxnArgs[NUM_DMA_CHANNELS];

static void DMAMSPM0_hwiFxn(uintptr_t arg);


/* Default DMA parameters structure */
const DMAMSPM0_Transfer DMA_Ch_defaultParams =
{
    .txTrigger = DMA_SOFTWARE_TRIG,  /* default is sw trigger */
    .rxTrigger = DMA_SOFTWARE_TRIG,  /* default is sw trigger */
    .txTriggerType = DL_DMA_TRIGGER_TYPE_EXTERNAL,
    .rxTriggerType = DL_DMA_TRIGGER_TYPE_EXTERNAL,
    .extendedMode = DL_DMA_NORMAL_MODE,  /* default is normal extended mode */
    .transferMode = DL_DMA_SINGLE_TRANSFER_MODE,        /* default is single transfer mode */
    .srcWidth  = DL_DMA_WIDTH_BYTE,  /* default src width is 32-bit */
    .destWidth = DL_DMA_WIDTH_BYTE,  /* default dest width is 32-bit */
    .srcIncrement = DL_DMA_ADDR_INCREMENT,  /* default is increment DMA src address */
    .destIncrement = DL_DMA_ADDR_INCREMENT,        /* default is increment DMA dest address */
    .dmaChannel             = 0,      /* default DMA ch is 0 */
    .dmaTransferSource      = NULL,   /* default source address is null */
    .dmaTransferDestination = NULL,   /* default destination address is null */
    .dmaChIsrFxn            = NULL,   /* default the DMA channel isr function is NULL */
    .enableDMAISR           = false,  /* default the DMA channel isr is disabled */
};

/**
 * @brief 关闭 DMA 通道并释放资源 / Close DMA channel and release resources
 * @param handle DMA 寄存器句柄
 * @param channel DMA 通道号
 */
void DMAMSPM0_close(DMA_Regs *handle, uint8_t channel)
{
    uintptr_t key;
    uint16_t channelMask;
    uint8_t channelNum;

    channelNum  = channel;
    channelMask = 1 << channelNum;

    /* 通道号越界检查 / Channel number range check */
    if (channelNum >= NUM_DMA_CHANNELS)
    {
        return;
    }

    /* 通道未使用则直接返回 / Return if channel is not in use */
    if ((channelsInUse & channelMask) == 0)
    {
        return;
    }

    if (channelNum < NUM_DMA_CHANNELS)
    {
        /* Acquire exclusive access to interrupt arrays & masks / 获取中断配置锁 */
        SemaphoreP_pend(interruptSetupLock, SemaphoreP_WAIT_FOREVER);

        /* 清除通道中断处理函数和参数 / Clear channel ISR handler and args */
        hwiFxnHandlers[channelNum] = NULL;
        hwiFxnArgs[channelNum]     = (uintptr_t) NULL;

        dmaIntRefCount--;

        /* 最后一个通道关闭时删除硬件中断句柄 / Delete Hwi when last channel closes */
        if (dmaIntRefCount == 0)
        {
            HwiP_delete(hwiHandle);
            hwiHandle = NULL;

            chTransferSet &= ~channelMask;
        }

        SemaphoreP_post(interruptSetupLock);
    }

    /* Free the channel / 释放通道 */
    key = HwiP_disable();

    channelsInUse &= ~channelMask;

    HwiP_restore(key);
}

/**
 * @brief 初始化 DMA 驱动（创建信号量和硬件中断）/ Initialize DMA driver (create semaphore and Hwi)
 * @return true 初始化成功，false 初始化失败
 */
bool DMAMSPM0_init(void)
{
    uintptr_t key;
    HwiP_Params hwiParams;
    SemaphoreP_Handle sem;
    DMAMSPM0_Handle handle    = (DMAMSPM0_Handle)&DMAMSPM0_Config;
    DMAMSPM0_HWAttrs *hwAttrs = (DMAMSPM0_HWAttrs *)handle->hwAttrs;

    /* speculatively create a binary semaphore for thread safety / 预创建二值信号量 */
    sem = SemaphoreP_createBinary(1);

    key = HwiP_disable();

    if (interruptSetupLock == NULL)
    {
        /* use the binary sem created above / 首次初始化，使用刚创建的信号量 */
        interruptSetupLock = sem;

        HwiP_restore(key);
    }
    else  /* if this function called again before DMA_Close(), then delete sem / 重复调用则删除多余信号量 */
    {
        /* delete unused Semaphore */
        if (sem)
        {
            SemaphoreP_delete(sem);
        }
        HwiP_restore(key);
        return false;
    }
     key = HwiP_disable();
    if (interruptSetupLock == NULL)  /* semaphore is not set / 信号量未创建 */
    {
        HwiP_restore(key);
        return false;
    }
    HwiP_restore(key);
    /* Acquire exclusive access while initializing / 获取独占访问锁 */
    SemaphoreP_pend(interruptSetupLock, SemaphoreP_WAIT_FOREVER);

    if (!dmaInitialized)
    {
        /* 设置 DMA 中断处理函数 / Set DMA ISR handler */
        hwAttrs->dmaIsrFxn = &DMAMSPM0_hwiFxn;

        /* 创建硬件中断 / Create DMA Hwi */
        HwiP_Params_init(&hwiParams);
        hwiParams.arg      = (uintptr_t) handle;
        hwiParams.priority = hwAttrs->intPriority;
        hwiHandle = HwiP_create(DMA_INT_VECn, hwAttrs->dmaIsrFxn, &hwiParams);

        if (hwiHandle == NULL)
        {
            SemaphoreP_post(interruptSetupLock);
            return false;
        }

        dmaInitialized = true;

        /* if round robin priority is not enabled before then enable it / 配置轮询优先级 */
        if ((hwAttrs->roundRobinPriority) && (!DL_DMA_isRoundRobinPriorityEnabled(DMA)))
        {
            DL_DMA_enableRoundRobinPriority(DMA);
        }
        else if (!(hwAttrs->roundRobinPriority) && (DL_DMA_isRoundRobinPriorityEnabled(DMA)))
        {
            DL_DMA_disableRoundRobinPriority(DMA);
        }
    }

    SemaphoreP_post(interruptSetupLock);
    return true;
}

/**
 * @brief 初始化 DMA 传输参数为默认值 / Initialize DMA transfer params to defaults
 * @param params DMA 传输参数指针
 */
void DMA_Params_init(DMAMSPM0_Transfer *params)
{
    *params = DMA_Ch_defaultParams;
}

/**
 * @brief 打开 DMA 通道并配置中断 / Open DMA channel and configure interrupt
 * @param index DMA 配置索引
 * @param channelNum DMA 通道号(0-3 或 0-7)
 * @return 成功返回 DMA 句柄，失败返回 NULL
 */
DMAMSPM0_Handle DMAMSPM0_open(uint_least8_t index, uint8_t channelNum)
{
    uintptr_t key;
    uint8_t channelNo;
    uint32_t channelMask;

    DMAMSPM0_Handle handle = (DMAMSPM0_Handle)DMAMSPM0_Config;
    DMAMSPM0_Transfer* object = ((DMAMSPM0_Transfer*)handle->object)+index;

    /* 驱动未初始化则返回失败 / Return NULL if driver not initialized */
    if (!dmaInitialized || hwiHandle == NULL)
    {
        /* Driver did not initialize correctly */
        return (NULL);
    }

    channelNo   = channelNum;
    channelMask = 1 << channelNo;

    /* 通道号越界检查 / Channel range check */
    if (channelNo >= NUM_DMA_CHANNELS)
    {
        return (NULL);
    }

    key = HwiP_disable();

    /*
     *  If the desired channel is already being used by another peripheral
     *  abort. Otherwise, claim the channel and set it up to transfer data.
     *  若通道已被占用则返回 NULL，否则标记为已使用
     */
    if (channelsInUse & channelMask)
    {
        HwiP_restore(key);
        return (NULL);
    }

    channelsInUse |= channelMask;

    /* Configure Hwi for DMA channel / 配置通道中断 */
    if (object->enableDMAISR && (object->dmaChIsrFxn != NULL))
    {
        /* Acquire exclusive access to interrupt arrays & masks / 获取中断配置锁 */
        SemaphoreP_pend(interruptSetupLock, SemaphoreP_WAIT_FOREVER);

        /* Clear DMA interrupt flags before creating the Hwi / 创建中断前清除标志 */
        DL_DMA_clearInterruptStatus(DMA, channelMask);

        /* 注册通道中断处理函数 / Register channel ISR */
        hwiFxnHandlers[channelNum] = object->dmaChIsrFxn;
        hwiFxnArgs[channelNum]     = (uintptr_t) handle;
        dmaIntRefCount++;

        SemaphoreP_post(interruptSetupLock);

        /* Setup the peripheral interrupt / 使能通道中断 */
        DL_DMA_enableInterrupt(DMA, channelMask);
    }

    HwiP_restore(key);

    return handle;
}

/**
 * @brief 配置并启动 DMA 通道传输 / Configure and start DMA channel transfer
 * @param transfer DMA 传输参数指针
 * @param DMACfg DMA 配置参数指针
 * @return true 成功，false 失败
 */
bool DMAMSPM0_setupTransfer(DMAMSPM0_Transfer *transfer, DL_DMA_Config* DMACfg)
{
    uintptr_t key;
    uint32_t chMask                   = 0;
    DMAMSPM0_Transfer *transferHandle = transfer;
    uint8_t dmaChannelIndex           = transferHandle->dmaChannel;
    chMask                            = 1 << dmaChannelIndex;

    /* The DMA must be open in order to configure a channel / DMA 必须已初始化 */
    if (!dmaInitialized)
    {
        return (false);
    }
    /* The channel transfer should not be set / 通道传输不应重复配置 */
    if (chTransferSet & chMask)
    {
        return (false);
    }

    /* A lock is needed because we are accessing shared DMA registers / 访问共享寄存器需关中断 */
    key = HwiP_disable();

    /* set source address / 设置源地址 */
    DL_DMA_setSrcAddr(DMA, dmaChannelIndex, (uint32_t)transferHandle->dmaTransferSource);

    /* set destination address / 设置目标地址 */
    DL_DMA_setDestAddr(DMA, dmaChannelIndex,(uint32_t)transferHandle->dmaTransferDestination);

    /* Initialize the channel for transfer / 初始化通道 */
    DL_DMA_initChannel(DMA, dmaChannelIndex, DMACfg);

    /* Set transfer Size / 设置传输大小 */
    DL_DMA_setTransferSize(DMA, dmaChannelIndex, transferHandle->noOfData);

    /* Enable the channel / 使能通道 */
    DL_DMA_enableChannel(DMA, dmaChannelIndex);

    /* setup for channel is done / 标记通道已配置 */
    chTransferSet |= chMask;

    /* Clear the channel interrupt / 清除通道中断 */
    DL_DMA_clearInterruptStatus(DMA, ((uint32_t)(1 << dmaChannelIndex)));

    /* start transfer / 启动传输 */
    DL_DMA_startTransfer(DMA, dmaChannelIndex);

    HwiP_restore(key);

    return (true);
}

/**
 * @brief 禁用指定 DMA 通道 / Disable specified DMA channel
 * @param handle DMA 配置句柄
 * @param dmaChannel DMA 通道号
 */
void DMAMSPM0_disableChannel(DMAMSPM0_Handle handle, uint8_t dmaChannel)
{
    uint32_t chMask = 1 << dmaChannel;
    /*Clear the channel interrupt / 清除通道中断 */
    DL_DMA_clearInterruptStatus(DMA, ((uint32_t)(1 << dmaChannel)));
    /* 禁用通道 / Disable channel */
    DL_DMA_disableChannel(DMA, dmaChannel);
    /* 清除传输配置标志 / Clear transfer setup flag */
    chTransferSet &= ~chMask;
}

/**
 * @brief 获取指定 DMA 通道的当前剩余传输字节数 / Get current transfer size for a DMA channel
 * @param dmaChannel DMA 通道号
 * @return 剩余传输字节数
 */
uint16_t DMAMSPM0_getCurrTransferSize(uint8_t dmaChannel)
{
    return (DL_DMA_getTransferSize(DMA, dmaChannel));
}

/**
 * @brief DMA 硬件中断处理函数 / DMA hardware interrupt handler
 * @param arg 中断参数(未使用)
 */
static void DMAMSPM0_hwiFxn(uintptr_t arg)
{
    uint8_t channelIndex;
    uint32_t dmaInterruptStatus;

    /* On each read, only one interrupt is indicated. On a read, the current
    * interrupt (highest priority) is automatically cleared by the hardware
    * and the corresponding interrupt flag in Section RIS and MIS
    * are cleared as well. After a read from the CPU ,
    * the register is updated with the next highest priority interrupt, if none are
    * pending, then it should display 0x0.
    * 每次读取仅返回一个中断（最高优先级），硬件自动清除对应标志。
    * 读取后寄存器更新为下一最高优先级中断，无待处理中断时返回 0x0。
    */
    dmaInterruptStatus = (uint32_t) DL_DMA_getPendingInterrupt(DMA);

    /* 循环处理所有待处理中断 / Loop until all pending interrupts are served */
    while (dmaInterruptStatus > 0)
    {
        /* Find which channel caused the interrupt and call its ISR / 查找触发中断的通道并调用其回调 */
        for (channelIndex = 0; channelIndex < NUM_DMA_CHANNELS; channelIndex++)
        {
            if (dmaInterruptStatus == channelIndex)
            {
                /* 清除通道中断标志 / Clear channel interrupt flag */
                DL_DMA_clearInterruptStatus(DMA, ((uint32_t)(1 << channelIndex)));
                if (hwiFxnHandlers[channelIndex])
                {
                    /* 调用通道回调函数 / Call channel ISR callback */
                    (*hwiFxnHandlers[channelIndex])(hwiFxnArgs[channelIndex]);
                    /*Once ISR is served, the transfer is done so clear the mask / 中断处理完毕，清除传输标志 */
                    chTransferSet &= ~((uint32_t)(1 << channelIndex));
                }
            }
        }
        /* keep checking interrupt until all are served / 继续检查是否有新的中断 */
        dmaInterruptStatus = (uint32_t) DL_DMA_getPendingInterrupt(DMA);
    }
}

/**
 * @brief 一键初始化 DMA（含所有通道）/ One-stop DMA initialization for all channels
 * @param dmaParams DMA 传输参数数组
 * @param DMACfg DMA 配置参数指针
 * @param noOfChs 使用的 DMA 通道数
 * @return 成功返回 DMA 句柄，失败返回 NULL
 */
DMAMSPM0_Handle DMA_Init(DMAMSPM0_Transfer* dmaParams, DL_DMA_Config* DMACfg, uint8_t noOfChs)
{
    uint8_t i = 0;
    DMAMSPM0_Handle dmaHandle;
    /* This init function will initialize the HWIP / 初始化 DMA 硬件中断 */
    DMAMSPM0_init();

    /* 逐通道初始化 / Initialize each channel */
    for(i = 0; i < noOfChs; i++)
    {
        /* Init with default params first / 先用默认参数初始化 */
        DMA_Params_init(&dmaParams[i]);
        /* Copy all User config DMA params to use for configuration / 从配置结构体复制用户参数 */
        memcpy(&dmaParams[i], ((DMAMSPM0_Transfer*)DMAMSPM0_Config[0].object)+i, sizeof(DMAMSPM0_Transfer));

        /* channel specific interrupt configuration / 打开通道并配置中断 */
        dmaHandle = DMAMSPM0_open(i, dmaParams[i].dmaChannel);

        if (dmaHandle == NULL)
        {
            return NULL;
        }
        /* 复制参数到 DL 配置结构体并初始化通道 / Copy params and init channel */
        DMAMSPM0_copyDMAParams(&dmaParams[i], DMACfg);
        DL_DMA_initChannel(DMA, dmaParams[i].dmaChannel, DMACfg);
    }
    return dmaHandle;
}

/**
 * @brief 从应用传输结构体复制参数到 DL 配置结构体 / Copy params from Transfer struct to DL config struct
 * @param dmaParams DMA 传输参数
 * @param dmaConfig DL DMA 配置结构体
 */
void DMAMSPM0_copyDMAParams(DMAMSPM0_Transfer* dmaParams, DL_DMA_Config* dmaConfig)
{
    /* Default value for trigger and triggerType are from RX / 默认使用 RX 触发配置 */
    dmaConfig->trigger      = dmaParams->rxTrigger;
    dmaConfig->triggerType  = dmaParams->rxTriggerType;
    dmaConfig->extendedMode = dmaParams->extendedMode;
    dmaConfig->srcWidth     = dmaParams->srcWidth;
    dmaConfig->srcIncrement = dmaParams->srcIncrement;
    dmaConfig->destWidth    = dmaParams->destWidth;
    dmaConfig->destIncrement = dmaParams->destIncrement;
    dmaConfig->transferMode = dmaParams->transferMode;
}
/* clang-format on */
