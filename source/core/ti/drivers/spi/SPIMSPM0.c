/*
 * Copyright (c) 2022-2023, Texas Instruments Incorporated
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
/* clang-format off */
/*
 * SPIMSPM0.c - MSPM0 SPI 驱动实现
 * 支持 DMA 传输、轮询传输、主机/从机模式、回调/阻塞模式。
 */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* Driverlib header files */
#include <ti/devices/DeviceFamily.h>
#include <ti/devices/msp/peripherals/hw_spi.h>
#include <ti/drivers/dma/DMAMSPM0.h>
#include <ti/drivers/dpl/ClockP.h>
#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#ifdef POWER_MANAGEMENT_MSPM0
#include <ti/drivers/Power.h>
#endif
#include <ti/drivers/GPIO.h>
#include <ti/drivers/SPI.h>
#include <ti/drivers/spi/SPIMSPM0.h>

/* DMA 单次最大传输帧数 */
#define MAX_DMA_TRANSFER_AMOUNT (1024)

#define SPI_DATASIZE_8      (8)     /* 8位数据帧 */
#define SPI_DATASIZE_16     (16)    /* 16位数据帧 */
#define PARAMS_DATASIZE_MIN SPI_DATASIZE_8  /* 最小数据帧大小 */
#define PARAMS_DATASIZE_MAX SPI_DATASIZE_16 /* 最大数据帧大小 */
#define DMA_CHANNELS (2)            /* DMA 通道数（TX + RX） */

/* API Function Prototypes */
void SPIMSPM0_close(SPI_Handle handle);
int_fast16_t SPIMSPM0_control(SPI_Handle handle, uint_fast16_t cmd, void *arg);
void SPIMSPM0_init(SPI_Handle handle);
SPI_Handle SPIMSPM0_open(SPI_Handle handle, SPI_Params *params);
bool SPIMSPM0_transfer(SPI_Handle handle, SPI_Transaction *transaction);
void SPIMSPM0_transferCancel(SPI_Handle handle);

/* Local Function Prototypes */
static void blockingTransferCallback(SPI_Handle handle, SPI_Transaction *msg);
static void configNextTransfer(SPIMSPM0_Object *object, SPIMSPM0_HWAttrs const *hwAttrs);
static void csnCallback(uint_least8_t);
static void flushFifos(SPIMSPM0_HWAttrs const *hwAttrs);
static void SPIMSPM0_hwiFxn(uintptr_t arg);
static bool initHw(SPI_Handle handle);
static void initIO(SPI_Handle handle);
static void finalizeIO(SPI_Handle handle);
static void setIOStandbyState(SPI_Handle handle);
static inline void primeTransfer(SPI_Handle handle);
static inline void spiPollingTransfer(SPI_Handle handle, SPI_Transaction *transaction);
#ifdef POWER_MANAGEMENT_MSPM0
static inline void releaseConstraint(uint32_t txBufAddr);
static inline void setConstraint(uint32_t txBufAddr);
static int spiPostNotify(uint32_t eventType, uintptr_t eventArg, uintptr_t clientArg);
#endif
static inline bool spiBusy(SPIMSPM0_Object *object, SPIMSPM0_HWAttrs const *hwAttrs);
static inline void disableSPI(SPIMSPM0_HWAttrs const *hwAttrs);
static inline void enableSPI(SPI_Handle handle);
static void enableDMA(SPIMSPM0_HWAttrs const *hwAttrs);
static void disableDMA(SPI_Handle handle);
static bool configSPI(SPI_Handle handle, uint32_t freq, uint32_t format);
static bool isDMATxDone(SPIMSPM0_Object* object);
static bool DMATxRxEnabled = false;
static void disableDMARx(SPI_Handle handle);
static void disableDMATx(SPI_Handle handle);

/* RX FIFO over flowed, data was lost */
#define SPI_INT_RXOF   (DL_SPI_INTERRUPT_RX_OVERFLOW)   /* RX FIFO 溢出，数据丢失 */
/* RX FIFO trigger level was passed */
#define SPI_INT_RXFF   (DL_SPI_INTERRUPT_RX)            /* RX FIFO 触发阈值达到 */
/* TX FIFO trigger level was passed */
#define SPI_INT_TXFF   (DL_SPI_INTERRUPT_TX)            /* TX FIFO 触发阈值达到 */
/* Transfer complete, HW is idle */
#define SPI_INT_IDLE   (DL_SPI_INTERRUPT_IDLE)          /* 传输完成，硬件空闲 */
/* Subset of SPI interrupts used by this driver */
#define SPI_INT_SUBSET (SPI_INT_TXFF | SPI_INT_RXFF | SPI_INT_RXOF | SPI_INT_IDLE)
/* All SPI interrupts */
#define SPI_INT_ALL                                                   \
    (SPI_INT_SUBSET | DL_SPI_INTERRUPT_TX_EMPTY |                     \
        DL_SPI_INTERRUPT_PARITY_ERROR | DL_SPI_INTERRUPT_RX_TIMEOUT | \
        DL_SPI_INTERRUPT_DMA_DONE_RX | DL_SPI_INTERRUPT_DMA_DONE_TX)

/* DSAMPLE default value limits based bit rate */
/* 采样延迟默认值阈值（基于比特率） */
#define DSAMPLE_MED_BITRATE  4000000    /* 4Mbps，延迟=1 */
#define DSAMPLE_HIGH_BITRATE 8000000    /* 8Mbps，延迟=2 */

/* Sampling delay value */
#define DSAMPLE_MAX_CYCLES 15           /* 最大采样延迟时钟周期数 */

/* Maximum serial clock divider value */
#define SERIAL_CLK_DIVIDER_MAX 0x3FFU   /* 串行时钟分频器最大值 */
/*! There are two channels considered here, one for tx and another for rx */
/* DMA 传输描述符数组，分别用于 TX 和 RX */
static DMAMSPM0_Transfer DMATransfer[DMA_CHANNELS];
static DL_DMA_Config dlDMACfg;              /* DMA 配置结构体 */

/* SPI function table for SPIMSPM0 implementation */
/* MSPM0 SPI 驱动函数表，注册到 SPI_Config 中 */
const SPI_FxnTable SPIMSPM0_fxnTable = {SPIMSPM0_close,
                                          SPIMSPM0_control,
                                          SPIMSPM0_init,
                                          SPIMSPM0_open,
                                          SPIMSPM0_transfer,
                                          SPIMSPM0_transferCancel};

/* SPI 工作模式映射表 */
static const uint32_t mode[] = {
    /* SPI_CONTROLLER */
    DL_SPI_MODE_CONTROLLER,     /* 主机模式 */
    /* SPI_PERIPHERAL */
    DL_SPI_MODE_PERIPHERAL,     /* 从机模式 */
};

/* Mapping SPI frame format from generic driver to driverlib */
/* SPI 帧格式映射表：通用枚举 -> driverlib 枚举 */
static const uint32_t frameFormat[] = {
    /* SPI_POL0_PHA0 */
    DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0,
    /* SPI_POL0_PHA1 */
    DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA1,
    /* SPI_POL1_PHA0 */
    DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA0,
    /* SPI_POL1_PHA1 */
    DL_SPI_FRAME_FORMAT_MOTO3_POL1_PHA1,
    DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA0,
    DL_SPI_FRAME_FORMAT_MOTO4_POL0_PHA1,
    DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA0,
    DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1,
    /* SPI_TI */
    (DL_SPI_FRAME_FORMAT_TI_SYNC),
};

/* 数据帧大小映射表：索引 0=4bit, 1=5bit, ..., 12=16bit */
static const uint32_t dataSize[] =
{
    DL_SPI_DATA_SIZE_4,
    DL_SPI_DATA_SIZE_5,
    DL_SPI_DATA_SIZE_6,
    DL_SPI_DATA_SIZE_7,
    DL_SPI_DATA_SIZE_8,
    DL_SPI_DATA_SIZE_9,
    DL_SPI_DATA_SIZE_10,
    DL_SPI_DATA_SIZE_11,
    DL_SPI_DATA_SIZE_12,
    DL_SPI_DATA_SIZE_13,
    DL_SPI_DATA_SIZE_14,
    DL_SPI_DATA_SIZE_15,
    DL_SPI_DATA_SIZE_16,
};

/*
 *  ======== SPIMSPM0_close ========
 */
/**
 * @brief 关闭 MSPM0 SPI 外设。
 * 禁用 SPI、销毁中断和信号量、释放 IO 和电源依赖。
 *
 * @param handle SPI 句柄
 */
void SPIMSPM0_close(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    /* Disable the SPI */
    DL_SPI_disable(hwAttrs->spi);       /* 禁用 SPI 外设 */

    HwiP_destruct(&(object->hwi));      /* 销毁硬件中断对象 */

    if (object->transferMode == SPI_MODE_BLOCKING)
    {
        SemaphoreP_destruct(&(object->transferComplete));  /* 销毁阻塞模式信号量 */
    }

    finalizeIO(handle);                 /* 释放 IO 引脚配置 */
#ifdef POWER_MANAGEMENT_MSPM0
    Power_releaseDependency(hwAttrs->powerID);     /* 释放电源依赖 */
    Power_unregisterNotify(&object->spiPostObj);    /* 注销电源通知 */
#endif
    object->isOpen = false;             /* 标记驱动已关闭 */
}

/*!
 *  ======== SPIMSPM0_control ========
 *  @brief  Function for setting control parameters of the SPI driver
 *          after it has been opened.
 *  在 SPI 驱动打开后设置控制参数，支持部分返回、片选切换、手动启动、采样延迟等。
 *
 *  @pre    SPIMSPM0_open() has to be called first.
 *          Calling context: Hwi, Task
 *
 *  @param  handle A SPI handle returned from SPIMSPM0_open()  SPI 句柄
 *
 *  @param  cmd  The command to execute  控制命令码
 *  | Command                                   | Description                  |
 *  |-------------------------------------------|------------------------------|
 *  | ::SPIMSPM0_CMD_RETURN_PARTIAL_ENABLE  | Enable RETURN_PARTIAL        |
 *  | ::SPIMSPM0_CMD_RETURN_PARTIAL_DISABLE | Disable RETURN_PARTIAL       |
 *  | ::SPIMSPM0_CMD_SET_CSN_PIN            | Re-configure chip select pin |
 *  | ::SPIMSPM0_CMD_SET_MANUAL             | Enable manual start mode     |
 *  | ::SPIMSPM0_CMD_CLR_MANUAL             | Disable manual start mode    |
 *  | ::SPIMSPM0_CMD_MANUAL_START           | Perform a manual start       |
 *  | ::SPIMSPM0_CMD_SET_SAMPLE_DELAY       | Set a custom DSAMPLE value   |
 *
 *  @param  *arg  Pointer to command arguments.  命令参数指针
 *
 *  @return ::SPI_STATUS_SUCCESS if success, or error code if error.
 *  @return 成功返回 SPI_STATUS_SUCCESS，失败返回错误码。
 */
int_fast16_t SPIMSPM0_control(SPI_Handle handle, uint_fast16_t cmd, void *arg)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint_least8_t pinIndex;
    uint_least8_t sampleDelay;

    /* Initialize return value */
    int ret = SPI_STATUS_ERROR;     /* 默认返回错误 */

    /* Perform command */
    switch (cmd)
    {
        case SPIMSPM0_CMD_RETURN_PARTIAL_ENABLE:
            /* Enable RETURN_PARTIAL if peripheral mode is enabled */
            /* 仅从机模式可启用部分返回 */

            if (object->mode == SPI_PERIPHERAL)
            {
                object->returnPartial = SPIMSPM0_retPartEnabledIntNotSet;
                ret                   = SPI_STATUS_SUCCESS;
            }
            else
            {
                /* Partial return not available in controller mode. */
                ret = SPI_STATUS_ERROR;
            }
            break;

        case SPIMSPM0_CMD_RETURN_PARTIAL_DISABLE:
            /* 禁用部分返回，清除片选中断 */
            GPIO_setInterruptConfig(object->csnPin, GPIO_CFG_IN_INT_NONE);
            object->returnPartial = SPIMSPM0_retPartDisabled;
            ret                   = SPI_STATUS_SUCCESS;
            break;

        case SPIMSPM0_CMD_SET_CSN_PIN:
            /* 重新配置片选引脚 */
            pinIndex = *((uint_least8_t *)arg);

            if (pinIndex == GPIO_INVALID_INDEX)
            {
                /* If trying to disable the CS pin, forward to the CLEAR_CS command and break */
                /* 索引无效则清除片选 */
                ret = SPIMSPM0_control(handle, SPIMSPM0_CMD_CLEAR_CSN_PIN, NULL);
                break;
            }

            /* Reset the previous CS pin and configure the new one */
            /* 重置旧片选引脚，配置新片选引脚 */
            GPIO_resetConfig(object->csnPin);
            object->csnPin = pinIndex;
            GPIO_setConfigAndMux(object->csnPin, GPIO_CFG_INPUT, hwAttrs->csnPinMux);

            ret = SPI_STATUS_SUCCESS;
            break;

        case SPIMSPM0_CMD_CLEAR_CSN_PIN:
            /* If the CS pin is assigned, reset and unassign it */
            /* 清除片选引脚配置 */
            if (object->csnPin != GPIO_INVALID_INDEX)
            {
                GPIO_resetConfig(object->csnPin);
                object->csnPin = GPIO_INVALID_INDEX;
            }
            ret = SPI_STATUS_SUCCESS;
            break;

        case SPIMSPM0_CMD_SET_MANUAL:
            /* If a transaction is queued, do not modify */
            /* 启用手动启动模式，有排队事务时不可修改 */
            if (object->headPtr == NULL)
            {
                object->manualStart = true;
                ret                 = SPI_STATUS_SUCCESS;
            }
            break;

        case SPIMSPM0_CMD_CLR_MANUAL:
            /* If a transaction is queued, do not modify */
            /* 禁用手动启动模式 */
            if (object->headPtr == NULL)
            {
                object->manualStart = false;
                ret                 = SPI_STATUS_SUCCESS;
            }
            break;

        case SPIMSPM0_CMD_MANUAL_START:
            /* 手动触发已排队事务的传输 */
            if (object->headPtr != NULL && object->manualStart)
            {
                enableSPI(handle);      /* 使能 SPI */
                enableDMA(hwAttrs);     /* 使能 DMA 触发 */
                DL_DMA_enableChannel(DMA, object->txDMAChannel);   /* 使能 TX DMA 通道 */
                DL_DMA_enableChannel(DMA, object->rxDMAChannel);   /* 使能 RX DMA 通道 */
                ret = SPI_STATUS_SUCCESS;
            }
            break;

        case SPIMSPM0_CMD_SET_SAMPLE_DELAY:
            /* 设置主机模式采样延迟 */
            sampleDelay = *((uint_least8_t *)arg);

            if (object->mode == SPI_CONTROLLER && sampleDelay <= DSAMPLE_MAX_CYCLES)
            {
                /* Cache the value so it is persistent across standby */
                object->dsample = sampleDelay << SPI_CLKCTL_DSAMPLE_OFS;    /* 缓存值以跨越待机保持 */

                DL_SPI_setDelayedSampling(hwAttrs->spi, sampleDelay);
                ret = SPI_STATUS_SUCCESS;
            }
            break;

        default:
            /* This command is not defined */
            /* 未定义的命令码 */
            ret = SPI_STATUS_UNDEFINEDCMD;
            break;
    }

    return (ret);
}

/*
 *  ======== SPIMSPM0_hwiFxn ========
 */
/**
 * @brief SPI 硬件中断服务函数。
 * 处理 RX 溢出、DMA 完成中断，管理事务队列和传输续传。
 *
 * @param arg SPI 句柄（通过 HWI 参数传入）
 */
static void SPIMSPM0_hwiFxn(uintptr_t arg)
{
    uint32_t intStatus = 0;
    size_t *transferSize;
    SPIMSPM0_Object *object         = ((SPI_Handle)arg)->object;
    SPIMSPM0_HWAttrs const *hwAttrs = ((SPI_Handle)arg)->hwAttrs;
    uint8_t i;

    intStatus = DL_SPI_getEnabledInterruptStatus(hwAttrs->spi, SPI_INT_ALL);  /* 读取中断状态 */
    DL_SPI_clearInterruptStatus(hwAttrs->spi, intStatus);                      /* 清除中断标志 */

    if (intStatus & SPI_INT_RXOF)  /* RX FIFO 溢出 */
    {
        if (object->headPtr != NULL)
        {
            /*
             * RX overrun during a transfer; mark the current transfer
             * as failed & cancel all remaining transfers.
             */
            /* 传输中发生 RX 溢出，标记当前传输失败并取消剩余事务 */
            object->headPtr->status = SPI_TRANSFER_FAILED;

            SPIMSPM0_transferCancel((SPI_Handle)arg);
        }
        else
        {
            /* Disable DMA and clear DMA interrupts */
            /* 无事务时溢出，禁用 DMA 和中断 */
            disableDMA((SPI_Handle)arg);
            DMAMSPM0_disableChannel(object->DMA_Handle, object->txDMAChannel);
            DMAMSPM0_disableChannel(object->DMA_Handle, object->rxDMAChannel);
            DL_SPI_disableInterrupt(hwAttrs->spi, SPI_INT_ALL);
            DL_SPI_clearInterruptStatus(hwAttrs->spi, SPI_INT_ALL);
        }
    }
    else
    {
        if (object->headPtr == NULL)
        {
            /* When i was 0, we finished the last transaction */
            /* 队列为空，最后一个事务已完成 */
            return;
        }

        transferSize = &object->transferSize;

        /*
         * The SPI TX FIFO continuously requests the DMA to fill it if there
         * is space available.  If there are no more frames to put in the
         * FIFO we run into a situation where DMA TX will cause undesired
         * interrupts.  To prevent many undesired interrupts disable DMA_TX
         * if there are no more frames to load into the FIFO & there are no
         * pending queued transactions.
         */
        /* TX DMA 完成且无后续事务时，禁用 TX DMA 避免无用中断 */
        if ((intStatus & DL_SPI_INTERRUPT_DMA_DONE_TX) && object->framesQueued == object->headPtr->count && object->headPtr->nextPtr == NULL)
        {
            disableDMATx((SPI_Handle)arg);
            DL_DMA_clearInterruptStatus(DMA, ((uint32_t)(1 << object->txDMAChannel)));
        }

        /* RX DMA 完成中断处理 */
        if ((intStatus & DL_SPI_INTERRUPT_DMA_DONE_RX) && *transferSize != 0)
        {
            disableDMARx((SPI_Handle)arg);                          /* 禁用 RX DMA */
            object->framesTransferred += *transferSize;             /* 累计已传输帧数 */
            DL_DMA_clearInterruptStatus(DMA, ((uint32_t)(1 << object->rxDMAChannel)));

            /*
             * Set the channel's transfer size to 0; 0 lets
             * configNextTransfer() know that there is a free channel.
             */
            *transferSize = 0;  /* 清零标记通道空闲 */

            if ((object->framesQueued) < (object->headPtr->count) || (object->framesTransferred) < (object->headPtr->count))
            {
                /*
                 * In this case we need to reconfigure the channel to
                 * continue transferring frames. configNextTransfer() will
                 * continue queueing frames for the current transfer or
                 * start the following transaction if necessary.
                 */
                /* 当前事务未完成，重新配置 DMA 续传剩余帧 */
                configNextTransfer(object, hwAttrs);
            }
            else
            {
                /* Update the transaction status */
                object->headPtr->status = SPI_TRANSFER_COMPLETED;   /* 标记传输完成 */

                /* Execute callback function for completed transfer */
                object->transferCallbackFxn((SPI_Handle)arg, object->headPtr);  /* 调用回调 */

                /* Move the object->headPtr to the next transaction */
                object->headPtr = object->headPtr->nextPtr;         /* 移动到下一个事务 */

                /* Update object variables for the following transfer. */
                object->framesQueued      = 0;
                object->framesTransferred = 0;

                if (object->headPtr != NULL)
                {
                    /* Reconfigure channel for following transaction */
                    configNextTransfer(object, hwAttrs);            /* 配置下一个事务 */
                }
                else
                {
                    /* No more queued transfers; disable DMA & SPI */
                    /* 无更多事务，禁用 DMA */
                    disableDMA((SPI_Handle)arg);

                    /*
                     * For this driver implementation the peripheral is kept
                     * active until either a FIFO-overrun occurs or
                     * SPI_transferCancel() is executed.
                     */
                }
            }
        }
    }
}

/*
 *  ======== SPIMSPM0_init ========
 */
/** @brief 初始化 MSPM0 SPI 驱动实例，将 isOpen 标志设为 false。 */
void SPIMSPM0_init(SPI_Handle handle)
{
    ((SPIMSPM0_Object *)handle->object)->isOpen = false;
}

/*
 *  ======== SPIMSPM0_open ========
 */
/**
 * @brief 打开 MSPM0 SPI 外设。
 * 配置硬件、初始化 IO、创建中断和信号量/DMA、设置电源依赖。
 *
 * @param handle SPI 配置句柄
 * @param params SPI 参数指针
 * @return 成功返回句柄，失败返回 NULL
 */
SPI_Handle SPIMSPM0_open(SPI_Handle handle, SPI_Params *params)
{
    HwiP_Params hwiParams;
    uint32_t key;
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint8_t i = 0;

    key = HwiP_disable();

    /* Failure conditions */
    /* 检查是否已打开或数据大小超范围 */
    if (object->isOpen || params->dataSize > PARAMS_DATASIZE_MAX || params->dataSize < PARAMS_DATASIZE_MIN)
    {
        HwiP_restore(key);

        return (NULL);
    }
    object->isOpen = true;

    HwiP_restore(key);

    /* 从参数中复制配置到驱动对象 */
    object->bitRate            = params->bitRate;
    object->dataSize           = params->dataSize;
    object->mode               = params->mode;
    object->transferMode       = params->transferMode;
    object->transferTimeout    = params->transferTimeout;
    object->dataSize           = params->dataSize;
    object->parity             = params->parity;
    object->bitOrder           = params->bitOrder;
    object->returnPartial      = SPIMSPM0_retPartDisabled;    /* 默认禁用部分返回 */
    object->headPtr            = NULL;
    object->tailPtr            = NULL;
    object->format             = params->frameFormat;
    object->txScratchBuf       = hwAttrs->defaultTxBufValue;  /* 空 txBuf 时的默认发送值 */
    object->busyBit            = (params->mode == SPI_CONTROLLER ? SPI_STAT_BUSY_ACTIVE : SPI_STAT_TFE_EMPTY);
    object->manualStart        = false;                       /* 默认禁用手动启动 */

    /*
     * Set a default dsample value based on the bitRate
     * DSAMPLE can be changed later by the user using the SPI_control() API with
     * the CMD_SET_SAMPLE_DELAY option.
     */
    object->dsample = 0;
    if (object->bitRate >= DSAMPLE_MED_BITRATE)
    {
        object->dsample = 1 << SPI_CLKCTL_DSAMPLE_OFS;
    }
    else if (object->bitRate >= DSAMPLE_HIGH_BITRATE)
    {
        object->dsample = 2 << SPI_CLKCTL_DSAMPLE_OFS;
    }

#ifdef POWER_MANAGEMENT_MSPM0
    /* Register power dependency - i.e. power up and enable clock for SPI. */
    Power_setDependency(hwAttrs->powerID);

    /* Register power dependency on UDMA driver */
    Power_setDependency(PowerMSPM0_PERIPH_DMA);
#endif

    /*
     * Configure IOs after hardware has been initialized so that IOs aren't
     * toggled unnecessary
     */
    if (!initHw(handle))
    {
        /*
         * Trying to use SPI driver when some other driver or application
         * has already allocated these pins, error!
         */
#ifdef POWER_MANAGEMENT_MSPM0
        Power_releaseDependency(hwAttrs->powerID);
#endif
        object->isOpen = false;
        return (NULL);
    }

    /* CS pin is initialized using hwAttrs, but can be re-configured later */
    object->csnPin = hwAttrs->csnPin;

    /*
     * IO configuration must occur before SPI IP is enabled
     * for peripheral mode.
     * For controller mode see enableSPI().
     */
    if (object->mode == SPI_PERIPHERAL)
    {
        initIO(handle);
    }

    HwiP_Params_init(&hwiParams);
    hwiParams.arg      = (uintptr_t)handle;
    hwiParams.priority = hwAttrs->intPriority;
    HwiP_construct(&(object->hwi), (int)hwAttrs->intNum, SPIMSPM0_hwiFxn, &hwiParams);

#ifdef POWER_MANAGEMENT_MSPM0
    Power_registerNotify(&object->spiPostObj,
                         PowerMSPM0_AWAKE_STANDBY | PowerMSPM0_ENTERING_STANDBY,
                         (Power_NotifyFxn)spiPostNotify,
                         (uint32_t)handle);
#endif

    if (object->transferMode == SPI_MODE_BLOCKING)
    {
        /*
         * Create a semaphore to block task execution for the duration of the
         * SPI transfer
         */
        /* 阻塞模式：创建二值信号量用于等待传输完成 */
        SemaphoreP_constructBinary(&(object->transferComplete), 0);
        object->transferCallbackFxn = blockingTransferCallback;
    }
    else
    {
        /* This function will return pointer DMA handle */
        /* 回调模式：初始化 DMA，注册用户回调函数 */
        object->DMA_Handle = DMA_Init(&DMATransfer[0], &dlDMACfg, hwAttrs->noOfDMAChannels);

        DebugP_assert(params->transferCallbackFxn != NULL);  /* 回调模式必须提供回调函数 */
        object->transferCallbackFxn = params->transferCallbackFxn;

        DL_SPI_enableInterrupt(hwAttrs->spi, (DL_SPI_INTERRUPT_DMA_DONE_RX |
                        DL_SPI_INTERRUPT_DMA_DONE_TX));     /* 使能 DMA 完成中断 */
    }
    /* Enable the HW interrupt */
    HwiP_enableInterrupt(hwAttrs->intNum);                   /* 使能硬件中断 */

    return (handle);
}

/*
 *  ======== SPIMSPM0_transfer ========
 */
/**
 * @brief 执行 MSPM0 SPI 传输事务。
 * 阻塞模式下使用轮询或 DMA+信号量等待；回调模式下使用 DMA 异步传输。
 *
 * @param handle      SPI 句柄
 * @param transaction 事务结构体指针
 * @return 成功启动返回 true，失败返回 false
 */
bool SPIMSPM0_transfer(SPI_Handle handle, SPI_Transaction *transaction)
{
    uint8_t alignMask;
    bool buffersAligned;
    uintptr_t key;
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (transaction->count == 0)
    {
        return (false);     /* 帧数为 0 直接返回失败 */
    }

    key = HwiP_disable();   /* 关闭中断，保护临界区 */

    /*
     * Make sure that the buffers are aligned properly.
     * alignMask is used to check if the RX/TX buffers addresses
     * are aligned to the frameSize.
     */
    if (object->dataSize <= SPI_DATASIZE_8)
    {
        alignMask = 0;
    }
    else
    {
        alignMask = 1;
    }

    buffersAligned = ((((uint32_t)transaction->rxBuf & alignMask) == 0) &&
                      (((uint32_t)transaction->txBuf & alignMask) == 0));

    /* 缓冲区未对齐或阻塞模式已有排队事务时返回失败 */
    if (!buffersAligned || (object->headPtr && object->transferMode == SPI_MODE_BLOCKING))
    {
        transaction->status = SPI_TRANSFER_FAILED;

        HwiP_restore(key);

        return (false);
    }
    else
    {
        if (object->headPtr)
        {
            /* 已有事务排队，将新事务追加到队列尾部 */
            object->tailPtr->nextPtr = transaction;
            object->tailPtr          = transaction;
            object->tailPtr->status  = SPI_TRANSFER_QUEUED;
        }
        else
        {
            /* 队列为空，作为首个事务入队 */
            object->headPtr = transaction;
            object->tailPtr = transaction;

            object->framesQueued      = 0;
            object->framesTransferred = 0;
            object->transferSize      = 0;
            object->tailPtr->status   = (object->returnPartial != SPIMSPM0_retPartDisabled)
                                            ? SPI_TRANSFER_PEND_CSN_ASSERT
                                            : SPI_TRANSFER_STARTED;
        }

        object->tailPtr->nextPtr = NULL;
    }

    /* In peripheral mode, optionally enable callback on CS de-assert */
    if (object->returnPartial == SPIMSPM0_retPartEnabledIntNotSet)
    {
        object->returnPartial = SPIMSPM0_retPartEnabledIntSet;
        GPIO_setInterruptConfig(object->csnPin, GPIO_CFG_IN_INT_BOTH_EDGES);
        GPIO_enableInt(object->csnPin);
    }
#ifdef POWER_MANAGEMENT_MSPM0
    /* Set constraints to guarantee transaction */
    setConstraint((uint32_t)transaction->txBuf);
#endif
    /*
     * Polling transfer if BLOCKING mode & transaction->count < threshold
     * Peripherals not allowed to use polling unless timeout is disabled
     * Polling not allowed with returnPartial mode
     */
    /* 阻塞模式 + 无部分返回 + （主机 或 超时禁用）时使用轮询传输 */
    if (object->transferMode == SPI_MODE_BLOCKING && object->returnPartial == SPIMSPM0_retPartDisabled &&
        (object->mode == SPI_CONTROLLER || object->transferTimeout == SPI_WAIT_FOREVER))
    {
        HwiP_restore(key);

        spiPollingTransfer(handle, transaction);    /* 轮询传输 */

#ifdef POWER_MANAGEMENT_MSPM0
        /* Release constraint since transaction is done */
        releaseConstraint((uint32_t)transaction->txBuf);
#endif

        /* Transaction completed; set status & mark SPI ready */
        object->headPtr->status = SPI_TRANSFER_COMPLETED;   /* 标记完成 */
        object->headPtr         = NULL;
        object->tailPtr         = NULL;
    }
    else
    {
        /*
         * Perform a DMA backed SPI transfer; we need exclusive access while
         * priming the transfer to prevent race conditions with
         * SPIMSPM0_transferCancel().
         */
        /* 使用 DMA 传输 */
        primeTransfer(handle);

        /* Enable the RX overrun interrupt in the SPI module */
        DL_SPI_enableInterrupt(hwAttrs->spi, SPI_INT_RXOF);    /* 使能 RX 溢出中断 */

        HwiP_restore(key);

        if (object->transferMode == SPI_MODE_BLOCKING)
        {
            /* 阻塞模式：等待信号量或超时 */
            if (SemaphoreP_OK != SemaphoreP_pend(&(object->transferComplete), object->transferTimeout))
            {
                /* Timeout occurred; cancel the transfer */
                /* 超时发生，取消传输 */
                object->headPtr->status = SPI_TRANSFER_FAILED;
                SPIMSPM0_transferCancel(handle);

                /*
                 * SPIMSPM0_transferCancel() performs callback which posts
                 * transferComplete semaphore. This call consumes this extra
                 * post.
                 */
                SemaphoreP_pend(&(object->transferComplete), SemaphoreP_NO_WAIT);

                return (false);
            }
        }
    }
    return (true);
}

/*
 *  ======== SPIMSPM0_transferCancel ========
 */
/**
 * @brief 取消正在进行的 SPI 传输。
 * 禁用 DMA 和 SPI，刷新 FIFO，更新队列中所有事务状态为 CANCELED，
 * 调用回调函数通知上层。
 *
 * @param handle SPI 句柄
 */
void SPIMSPM0_transferCancel(SPI_Handle handle)
{
    uintptr_t key;
    uint32_t temp;
    SPI_Transaction *tempPtr;
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    /*
     * Acquire exclusive access to the driver. Required to prevent race
     * conditions if preempted by code trying to configure another transfer.
     */
    key = HwiP_disable();   /* 关闭中断获取独占访问 */

    if (object->headPtr == NULL)
    {

        /*
         * Disable the SPI peripheral in case the peripherals finite state
         * machine is in a bad state. Calling SPI_transfer() will re-enable
         * the peripheral.
         */
        /* 无排队事务，禁用 SPI 防止状态机异常 */
        DL_SPI_disable(hwAttrs->spi);
        HwiP_restore(key);

        return;
    }

    /*
     * There are 2 use cases in which to call transferCancel():
     *   1.  The driver is in CALLBACK mode.
     *   2.  The driver is in BLOCKING mode & there has been a transfer timeout.
     */
    /* 回调模式或阻塞超时/CSN释放时执行取消 */
    if (object->transferMode != SPI_MODE_BLOCKING || object->headPtr->status == SPI_TRANSFER_FAILED ||
        object->headPtr->status == SPI_TRANSFER_CSN_DEASSERT)
    {

        /* Prevent interrupt while canceling the transfer */
        HwiP_disableInterrupt(hwAttrs->intNum);        /* 禁用中断防止竞争 */

        /*
         * Disable the TX DMA channel first to stop feeding more frames to
         * the FIFO.  Next, wait until the TX FIFO is empty (all frames in
         * FIFO have been sent).  RX DMA channel is disabled later to allow
         * the DMA to move all frames already in FIFO to memory.
         */
        /* 先禁用 TX DMA 停止向 FIFO 送数据 */
        DMAMSPM0_disableChannel(object->DMA_Handle, object->txDMAChannel);
        if (object->mode == SPI_CONTROLLER)
        {
            /*
             * Wait until the TX FIFO is empty; this is to make sure the
             * chip select is deasserted before disabling the SPI.
             */
            /* 主机模式等待 TX FIFO 清空，确保片选释放 */
            while (DL_SPI_isBusy(hwAttrs->spi)) {}
        }

        disableSPI(hwAttrs);    /* 禁用 SPI */

        /* Now disable the RX, DMA & interrupts */
        /* 禁用 RX DMA、DMA 触发和所有中断 */
        DMAMSPM0_disableChannel(object->DMA_Handle, object->rxDMAChannel);
        disableDMA(handle);
        DL_SPI_disableInterrupt(hwAttrs->spi, SPI_INT_ALL);
        DL_SPI_clearInterruptStatus(hwAttrs->spi, SPI_INT_ALL);

        /*
         * Update transaction->count with the amount of frames which have
         * been transferred.
         */
        /* 更新已传输帧数 */
        object->headPtr->count = object->framesTransferred;
        if (object->transferSize)
        {
            temp = DL_DMA_getTransferSize(DMA, object->txDMAChannel);

            if (temp <= object->transferSize)
            {
                object->headPtr->count += temp;
            }
        }

        /*
         * Disables peripheral, clears all registers & reinitializes it to
         * parameters used in SPI_open()
         */
        initHw(handle);     /* 重新初始化硬件 */

        HwiP_clearInterrupt(hwAttrs->intNum);
        HwiP_enableInterrupt(hwAttrs->intNum);

        /*
         * Go through all queued transfers; set status CANCELED (if we did
         * not cancel due to timeout).  The object->headPtr->count is
         * stored/restored temporarily.
         */
        /* 遍历队列，将所有非失败/非CSN释放的事务标记为 CANCELED */
        temp    = object->headPtr->count;
        tempPtr = object->headPtr;

        while (tempPtr != NULL)
        {
            if (tempPtr->status != SPI_TRANSFER_FAILED && tempPtr->status != SPI_TRANSFER_CSN_DEASSERT)
            {
                tempPtr->status = SPI_TRANSFER_CANCELED;
            }

            tempPtr->count = 0;
            tempPtr        = tempPtr->nextPtr;
        }
        object->headPtr->count = temp;

        /* Execute callback function for completed transfer */
        object->transferCallbackFxn((SPI_Handle)handle, object->headPtr);  /* 调用回调 */

        /* Clear all driver object variables*/
        /* 清除所有运行时变量 */
        object->headPtr           = NULL;
        object->tailPtr           = NULL;
        object->framesQueued      = 0;
        object->framesTransferred = 0;
        object->transferSize      = 0;

        HwiP_restore(key);

        /* Must return here; do not call HwiP_restore() twice */
        return;
    }

    HwiP_restore(key);
}

/*
 *  ======== blockingTransferCallback ========
 */
/** @brief 阻塞模式回调，释放传输完成信号量唤醒等待的任务。 */
static void blockingTransferCallback(SPI_Handle handle, SPI_Transaction *msg)
{
    SPIMSPM0_Object *object = handle->object;

    SemaphoreP_post(&(object->transferComplete));   /* 释放信号量 */
}

/*
 *  ======== configNextTransfer ========
 *  This function must be executed with interrupts disabled.
 */
/**
 * @brief 配置下一次 DMA 传输。
 * 根据当前事务进度分配 DMA 通道，设置 TX/RX 缓冲区和传输量。
 * 必须在中断禁用状态下执行。
 *
 * @param object  驱动对象指针
 * @param hwAttrs 硬件属性指针
 */
static void configNextTransfer(SPIMSPM0_Object *object, SPIMSPM0_HWAttrs const *hwAttrs)
{
    size_t framesQueued;
    uint32_t transferAmt;
    SPI_Transaction *transaction;
    uint8_t optionsIndex;

    /*
     * The DMA options vary according to data frame size; options for 8-bit
     * data (or smaller) are in optionsIndex 1.  Options for 16-bit frame sizes are
     * in optionsIndex 2.
     */
    /* 根据数据帧大小选择 DMA 地址增量：8位=1字节，16位=2字节 */
    if (object->dataSize <= SPI_DATASIZE_8)
    {
        optionsIndex = 1;
    }
    else
    {
        optionsIndex = 2;
    }

    /*
     * object->framesQueued keeps track of how many frames (of the current
     * transaction) have been configured for DMA transfer.  If
     * object->framesQueued == transaction->count; all frames have been queued
     * & we should configure the free DMA channel to send the next transaction.
     * When the current transaction has completed; object->framesQueued
     * will be updated (in the ISR) to reflect the amount of frames queued
     * of the following transaction.
     */
    transaction = object->headPtr;
    if (object->framesQueued < transaction->count)
    {
        framesQueued = object->framesQueued;
    }
    else
    {
        transaction = object->headPtr->nextPtr;
        if (transaction == NULL)
        {
            /* There are no queued transactions */
            return;
        }

        framesQueued        = 0;
        transaction->status = SPI_TRANSFER_STARTED;
    }

    /*
     * The DMA has a max transfer amount configured of 1024.  If the transaction is
     * greater; we must transfer it in chunks.  framesQueued keeps track of
     * how much data has been queued for transfer.
     */
    /* DMA 单次最大 1024 帧，超出时分块传输 */
    if ((transaction->count - framesQueued) > MAX_DMA_TRANSFER_AMOUNT)
    {
        transferAmt = MAX_DMA_TRANSFER_AMOUNT;
    }
    else
    {
        transferAmt = transaction->count - framesQueued;
    }

    /* Determine free channel & mark it as used by setting transfer size */
    if (object->transferSize == 0)
    {
        object->transferSize = transferAmt;
    }

    /* Setup the TX transfer buffers & characteristics */
    if (transaction->txBuf)
    {
        dlDMACfg.srcIncrement = DL_DMA_ADDR_INCREMENT;
        /*
         * Add an offset for the amount of data transferred.  The offset is
         * calculated by: object->framesQueued * (optionsIndex).  This
         * accounts for 8 or 16-bit sized transfers.
         */
        DMATransfer[0].dmaTransferSource = (uint8_t *)((uint32_t)transaction->txBuf + (uint32_t)(framesQueued * optionsIndex));
    }
    else
    {
        dlDMACfg.srcIncrement         = DL_DMA_ADDR_UNCHANGED;
        DMATransfer[0].dmaTransferSource = (uint8_t *) &(object->txScratchBuf);
    }
    object->txDMAChannel   = DMATransfer[0].dmaChannel;
    dlDMACfg.trigger       = DMATransfer[0].txTrigger;
    dlDMACfg.triggerType   = DMATransfer[0].txTriggerType;

    dlDMACfg.destIncrement        = DL_DMA_ADDR_UNCHANGED;
    DMATransfer[0].dmaTransferDestination = (uint32_t *) (&hwAttrs->spi->TXDATA);
    DMATransfer[0].noOfData     = transferAmt;
    DL_SPI_enableInterrupt(hwAttrs->spi, DL_SPI_INTERRUPT_DMA_DONE_TX);
    /* set DMA Tx transfer */
    DMAMSPM0_setupTransfer(&DMATransfer[0], &dlDMACfg);

    /* Setup the RX transfer buffers & characteristics */
    if (transaction->rxBuf)
    {
        /* It will increment the destination with the width of the data */
        dlDMACfg.destIncrement = DL_DMA_ADDR_INCREMENT;
        /*
         * Add an offset for the amount of data transferred.  The offset is
         * calculated by: object->framesQueued * (optionsIndex).  This
         * accounts for 8 or 16-bit sized transfers.
         */
        DMATransfer[1].dmaTransferDestination = (uint8_t *)((uint32_t)transaction->rxBuf + (uint32_t)(framesQueued * optionsIndex));
    }
    else
    {
        dlDMACfg.destIncrement         = DL_DMA_ADDR_UNCHANGED;
        DMATransfer[1].dmaTransferDestination = (uint8_t *)(&object->rxScratchBuf);
    }
    object->rxDMAChannel         = DMATransfer[1].dmaChannel;
    dlDMACfg.trigger             = DMATransfer[1].rxTrigger;
    dlDMACfg.triggerType         = DMATransfer[1].rxTriggerType;

    dlDMACfg.srcIncrement        = DL_DMA_ADDR_UNCHANGED;
    DMATransfer[1].dmaTransferSource = (uint32_t*) (&hwAttrs->spi->RXDATA);
    DMATransfer[1].noOfData     = transferAmt;
    DL_SPI_enableInterrupt(hwAttrs->spi, DL_SPI_INTERRUPT_DMA_DONE_RX);
    /* set DMA Rx transfer */
    DMAMSPM0_setupTransfer(&DMATransfer[1], &dlDMACfg);

    if (transaction == object->headPtr)
    {
        /*
         * Only update object->framesQueued if we are configuring a DMA
         * channel for the current transaction.
         */
        object->framesQueued += transferAmt;
    }

    /* Enable SPI TX interrupt as a trigger for DMA */
    DL_SPI_enableDMATransmitEvent(hwAttrs->spi);
    /* Enable SPI RX interrupt as a trigger for DMA */
    DL_SPI_enableDMAReceiveEvent(hwAttrs->spi, DL_SPI_DMA_INTERRUPT_RX);
}

/*
 *  ======== csnCallback ========
 *  Peripheral mode optional callback function for when the CS is asserted &
 *  deasserted.
 */
/**
 * @brief 从机模式片选引脚中断回调。
 * CS 拉低时标记传输开始，CS 拉高时取消传输（部分返回模式）。
 *
 * @param index GPIO 引脚索引
 */
static void csnCallback(uint_least8_t index)
{
    uintptr_t key;
    SPI_Handle spiHandle    = (SPI_Handle) GPIO_getUserArg(index);
    SPIMSPM0_Object *object = spiHandle->object;

    if (!GPIO_read(index))
    {
        /* Start transfer if CS goes low */
        key = HwiP_disable();
        if (object->headPtr != NULL)
        {
            /* We have data ready; indicate transaction started */
            object->headPtr->status = SPI_TRANSFER_STARTED;
        }
        else
        {
            /* We are not waiting to transfer; disable all interrupts */
            GPIO_setInterruptConfig(object->csnPin, GPIO_CFG_IN_INT_NONE);
            object->returnPartial = SPIMSPM0_retPartEnabledIntNotSet;
        }
        HwiP_restore(key);
    }
    else
    {
        /* Cancel ongoing transfer if CS high */
        key = HwiP_disable();

        GPIO_setInterruptConfig(object->csnPin, GPIO_CFG_IN_INT_NONE);
        object->returnPartial = SPIMSPM0_retPartEnabledIntNotSet;

        /* Indicate why the transaction completed */
        if (object->headPtr != NULL)
        {
            object->headPtr->status = SPI_TRANSFER_CSN_DEASSERT;
        }

        HwiP_restore(key);

        /* Cancel the current transaction */
        SPIMSPM0_transferCancel(spiHandle);
    }
}

/*
 *  ======== flushFifos ========
 *  This function writes 0s to the FIFOs but set all related interrupts as well
 *  during the SPI communication, it sends this 0s out first from the FIFO
 */
/**
 * @brief 刷新 SPI TX/RX FIFO。
 * 向 TX FIFO 写入 0 并从 RX FIFO 读出数据丢弃。
 */
static void flushFifos(SPIMSPM0_HWAttrs const *hwAttrs)
{
    uint16_t Buff[4] = {0, 0, 0, 0};
    /* Disable the SPI in case it isn't already */
    disableSPI(hwAttrs);
    /* clear the tx fifo */
    DL_SPI_fillTXFIFO16(hwAttrs->spi, Buff, 4);
    /* Clear the rx fifo */
    /* We will read the value in the local buffer
     * and these values will be discarded
     */
    DL_SPI_drainRXFIFO16(hwAttrs->spi, Buff, 4);
}

/*
 *  ======== initHw ========
 */
/**
 * @brief 初始化 SPI 硬件。
 * 禁用 SPI、清除中断、配置帧格式和时钟。
 *
 * @param handle SPI 句柄
 * @return 成功返回 true，失败返回 false
 */
static bool initHw(SPI_Handle handle)
{
    ClockP_FreqHz freq              = {0, 0};
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint32_t format;

    /* Disable SSI operation */
    DL_SPI_disable(hwAttrs->spi);

    /* Disable SPI module interrupts */
    DL_SPI_disableInterrupt(hwAttrs->spi, SPI_INT_ALL);
    DL_SPI_clearInterruptStatus(hwAttrs->spi, SPI_INT_ALL);

    /* Get requested format */
    format = frameFormat[object->format];

    if (hwAttrs->csnPin && ((format & SPI_CTL0_FRF_MASK) == SPI_CTL0_FRF_MOTOROLA_3WIRE))
    {
        /*
         * A CS pin was specified, upgrade the 3WIRE to a 4WIRE mode
         */
        format |= SPI_CTL0_FRF_MOTOROLA_4WIRE;
    }

    DL_SPI_setFrameFormat(hwAttrs->spi, format);

    /* Set the SPI configuration */
    ClockP_getCpuFreq(&freq);

    if (freq.lo / 2 >= object->bitRate)
    {
        configSPI(handle, freq.lo, format);
        return true;
    }
    return false;
}

/*
 *  ======== initIO ========
 *  This functions initializes the SPI IOs.
 *
 *  @pre    Function assumes that the SPI handle is pointing to a hardware
 *          module which has already been opened.
 */
/**
 * @brief 初始化 SPI IO 引脚。
 * 从机模式：SCLK/PICO/CSN 输入，POCI 输出；主机模式相反。
 *
 * @param handle SPI 句柄
 */
static void initIO(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (object->mode == SPI_PERIPHERAL)
    {
        GPIO_setConfigAndMux(hwAttrs->sclkPin, GPIO_CFG_INPUT | hwAttrs->sclkPincm, hwAttrs->sclkPinMux);
        GPIO_setConfigAndMux(hwAttrs->picoPin, GPIO_CFG_INPUT | hwAttrs->picoPincm, hwAttrs->picoPinMux);
        GPIO_setConfigAndMux(hwAttrs->pociPin, GPIO_CFG_OUTPUT | hwAttrs->pociPincm, hwAttrs->pociPinMux);
        GPIO_setConfigAndMux(hwAttrs->csnPin, GPIO_CFG_INPUT | hwAttrs->csnPincm, hwAttrs->csnPinMux);

        /* In peripheral mode, enable WU from STDBY on CSN */
        GPIO_setCallback(object->csnPin, csnCallback);
        GPIO_setUserArg(object->csnPin, handle);
    }
    else
    {
        GPIO_setConfigAndMux(hwAttrs->sclkPin, GPIO_CFG_OUTPUT | hwAttrs->sclkPincm, hwAttrs->sclkPinMux);
        GPIO_setConfigAndMux(hwAttrs->picoPin, GPIO_CFG_OUTPUT | hwAttrs->picoPincm, hwAttrs->picoPinMux);
        GPIO_setConfigAndMux(hwAttrs->pociPin, GPIO_CFG_INPUT | hwAttrs->pociPincm, hwAttrs->pociPinMux);
        GPIO_setConfigAndMux(hwAttrs->csnPin, GPIO_CFG_OUTPUT | GPIO_CFG_OUT_HIGH | GPIO_CFG_OUT_OD_PU | hwAttrs->csnPincm, hwAttrs->csnPinMux);
    }
}

/*
 *  ======== finalizeIO ========
 *  This function releases the SPI IOs.
 */
/**
 * @brief 释放 SPI IO 引脚，恢复 GPIO 默认配置。
 */
static void finalizeIO(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    GPIO_resetConfig(hwAttrs->picoPin);
    GPIO_resetConfig(hwAttrs->pociPin);
    GPIO_resetConfig(hwAttrs->sclkPin);

    /* We always mux CS in peripheral mode, but as controller it can be SW-controlled (and therefore unmuxed) */
    if (object->mode == SPI_PERIPHERAL || object->csnPin != GPIO_INVALID_INDEX)
    {
        GPIO_resetConfig(object->csnPin);
    }
}

/*
 *  ======== setIOStandbyState ========
 *  This function releases the SPI IOs and resets pins back to initial user and GPIO Configurations.
 */
/**
 * @brief 设置 SPI IO 引脚为待机状态，降低功耗。
 * 主机模式：CSN 高电平、SCLK 按极性设置、PICO 输入无上拉、POCI 输出低。
 * 从机模式：POCI 输出低。
 */
static void setIOStandbyState(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    bool sclkIsActiveHigh           = (object->format & SPI_CTL0_SPO_MASK) != 0;

    /* CONFIG_GPIO_SPI_CONTROLLER_SCLK */
    GPIO_PinConfig sclkPinConfig = GPIO_CFG_OUTPUT;

    /* CONFIG_GPIO_SPI_CONTROLLER_CSN */
    GPIO_PinConfig csnPinConfig = GPIO_CFG_OUTPUT | GPIO_CFG_OUT_HIGH;

    /* CONFIG_GPIO_SPI_CONTROLLER_POCI */
    GPIO_PinConfig picoPinConfig = GPIO_CFG_INPUT | GPIO_CFG_IN_NOPULL;

    /* CONFIG_GPIO_SPI_CONTROLLER_PICO */
    GPIO_PinConfig pociPinConfig = GPIO_CFG_OUTPUT | GPIO_CFG_OUT_LOW;

    if (object->mode == SPI_CONTROLLER)
    {
        GPIO_setConfig(object->csnPin, csnPinConfig);
        GPIO_setConfig(hwAttrs->picoPin, picoPinConfig);

        sclkPinConfig |= sclkIsActiveHigh ? GPIO_CFG_OUT_HIGH : GPIO_CFG_OUT_LOW;
        GPIO_setConfig(hwAttrs->sclkPin, sclkPinConfig);
    }
    else
    {
        GPIO_setConfig(hwAttrs->pociPin, pociPinConfig);
    }
}

/*
 *  ======== primeTransfer ========
 *  Function must be executed with interrupts disabled.
 */
/**
 * @brief 准备并启动 DMA 传输。
 * 配置 DMA 通道，使能 SPI 和 DMA。必须在中断禁用状态下执行。
 */
static inline void primeTransfer(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (object->transferSize > 0)
    {
        /* The transfer is in progress */
        /* 传输已在进行中，直接返回 */
        return;
    }

    if (object->transferSize != 0)
    {
        /*
         * Both channels are configured for a transfer.
         * In this case no work is required; the Hwi will configure channels
         * as transfers continue & complete.
         */
    }
    else if (object->transferSize == 0)
    {
        /*
         * Channels are disabled; no active transfer,
         * configure a new transfer.
         */
        /* 通道空闲，配置新的传输 */
        HwiP_disableInterrupt(hwAttrs->intNum);
        DL_SPI_enableInterrupt(hwAttrs->spi, (DL_SPI_INTERRUPT_DMA_DONE_RX |
                DL_SPI_INTERRUPT_DMA_DONE_TX));

        HwiP_clearInterrupt(hwAttrs->intNum);

        /* Configure RX & TX DMA transfers */
        configNextTransfer(object, hwAttrs);

        /* Enable the interrupts after configuring the transfer */
        HwiP_enableInterrupt(hwAttrs->intNum);

        if (!object->manualStart)
        {
            /* Enable DMA to generate interrupt on SPI peripheral */
            enableDMA(hwAttrs);
            /* Enable the SPI */
            enableSPI(handle);      /* 非手动模式立即启动 */
        }
    }
}
#ifdef POWER_MANAGEMENT_MSPM0
/*
 *  ======== releaseConstraint ========
 */
/** @brief 传输完成后释放电源约束。若 TX 缓冲区在 Flash 中，释放 Flash 保持约束。 */
static inline void releaseConstraint(uint32_t txBufAddr)
{
    /* Release need flash if buffer was in flash. */
    if (((txBufAddr & 0xF0000000) == 0x0) && (txBufAddr))
    {
        Power_releaseConstraint(PowerMSPM0_NEED_FLASH_IN_IDLE);
    }

    /* Release standby constraint since operation is done. */
    Power_releaseConstraint(PowerMSPM0_DISALLOW_STANDBY);
}

/*
 *  ======== setConstraint ========
 */
/** @brief 传输前设置电源约束。若 TX 缓冲区在 Flash 中，确保空闲时 Flash 可用。 */
static inline void setConstraint(uint32_t txBufAddr)
{
    /*
     * Ensure flash is available if TX buffer is in flash.
     * Flash starts with 0x0..
     */
    if (((txBufAddr & 0xF0000000) == 0x0) && (txBufAddr))
    {
        Power_setConstraint(PowerMSPM0_NEED_FLASH_IN_IDLE);
    }

    /* Set constraints to guarantee operation */
    Power_setConstraint(PowerMSPM0_DISALLOW_STANDBY);
}
#endif
/*
 *  ======== spiPollingTransfer ========
 */
/**
 * @brief 轮询方式执行 SPI 传输。
 * 逐帧填充 TX FIFO 并阻塞读取 RX FIFO，适用于小数据量低延迟场景。
 *
 * @param handle      SPI 句柄
 * @param transaction 事务结构体指针
 */
static inline void spiPollingTransfer(SPI_Handle handle, SPI_Transaction *transaction)
{
    uint8_t txIncrement, rxIncrement;
    size_t bytesCopiedToTxFifo;
    uint32_t dummyBuffer;
    size_t rxCount, txCount;
    void *rxBuf;
    void *txBuf;
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    rxIncrement = sizeof(uint8_t);
    txIncrement = sizeof(uint8_t);

    /* Only increment src/destination pointers if buffers were provided */
    if (transaction->rxBuf)
    {
        rxBuf = transaction->rxBuf;
        if (object->dataSize > SPI_DATASIZE_8)
        {
            rxIncrement = sizeof(uint16_t);
        }
    }
    else
    {
        rxBuf       = (void *) &(object->rxScratchBuf);
        rxIncrement = 0;
    }

    if (transaction->txBuf)
    {
        txBuf = transaction->txBuf;
        if (object->dataSize > SPI_DATASIZE_8)
        {
            txIncrement = sizeof(uint16_t);
        }
    }
    else
    {
        txBuf       = (void *) &(object->txScratchBuf);
        txIncrement = 0;
    }
    /* Number of frames(4to16bit per frame) for this transaction */
    rxCount = transaction->count;
    txCount = rxCount;

    enableSPI(handle);

    /* Fill the TX FIFO as much as we can before reading */
    while (rxCount--)
    {
        if (object->dataSize <= SPI_DATASIZE_8)
        {
            if(txCount > 0)
            {
                bytesCopiedToTxFifo = DL_SPI_fillTXFIFO8(hwAttrs->spi, (uint8_t *)txBuf, txCount);
                txBuf = (void *)(((uint32_t)txBuf) + bytesCopiedToTxFifo);
                txCount -= bytesCopiedToTxFifo;
            }

            dummyBuffer = DL_SPI_receiveDataBlocking8(hwAttrs->spi);
            *((uint8_t *)rxBuf) = (uint8_t)dummyBuffer;
        }
        else
        {
            if(txCount > 0)
            {
                bytesCopiedToTxFifo = DL_SPI_fillTXFIFO16(hwAttrs->spi, (uint16_t *)txBuf, txCount);
                txBuf = (void *)(((uint32_t)txBuf) + bytesCopiedToTxFifo);
                txCount -= bytesCopiedToTxFifo;
            }

            dummyBuffer = DL_SPI_receiveDataBlocking16(hwAttrs->spi);
            *((uint16_t *)rxBuf) = (uint16_t)dummyBuffer;
        }

        /* Update rxBuf position */
        rxBuf = (void *)(((uint32_t)rxBuf) + rxIncrement);
    }

    while (spiBusy(object, hwAttrs)) {}
}
#ifdef POWER_MANAGEMENT_MSPM0
/*
 *  ======== spiPostNotify ========
 */
/**
 * @brief 电源状态变化通知回调。
 * 进入待机时设置 IO 待机状态，唤醒后重新初始化硬件。
 */
static int spiPostNotify(uint32_t eventType, uintptr_t eventArg, uintptr_t clientArg)
{
    SPI_Handle handle       = (SPI_Handle)clientArg;
    SPIMSPM0_Object *object = handle->object;

    if (eventType == PowerMSPM0_ENTERING_STANDBY)
    {
        /* Set the SPI IO back to default state before entering STDBY */
        setIOStandbyState(handle);
    }
    else
    {
        initHw((SPI_Handle)clientArg);
        if (object->mode == SPI_PERIPHERAL)
        {
            initIO(handle);
        }
    }

    return (Power_NOTIFYDONE);
}
#endif
/*
 *  ======== spiBusy ========
 *  HW is busy when in controller mode and BSY bit is set, or when in peripheral mode
 *  and TFE bit is not set.
 */
/**
 * @brief 检查 SPI 是否忙。
 * 主机模式检查 BSY 位，从机模式检查 TX FIFO 是否非空。
 */
static inline bool spiBusy(SPIMSPM0_Object *object, SPIMSPM0_HWAttrs const *hwAttrs)
{
    /* In the controller mode, check the busy bit */
    if (object->busyBit == SPI_STAT_BUSY_ACTIVE)
    {
        return (DL_SPI_isBusy(hwAttrs->spi));
    }
    /* In the peripheral mode, check TFE is not empty */
    else
    {
        return (!DL_SPI_isTXFIFOEmpty(hwAttrs->spi));
    }
}

/*
 *  ======== disableSPI ========
 *  Disables the SPI peripheral
 */
/** @brief 禁用 SPI 外设。 */
static inline void disableSPI(SPIMSPM0_HWAttrs const *hwAttrs)
{
    DL_SPI_disable(hwAttrs->spi);
}

/*
 *  ======== enableSPI ========
 *  Enables the SPI peripheral and performs IO muxing as necessary
 */
/** @brief 使能 SPI 外设，主机模式下同时初始化 IO 复用。 */
static inline void enableSPI(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;


    if (!DL_SPI_isEnabled(hwAttrs->spi))
    {
        DL_SPI_enable(hwAttrs->spi);
        if (object->mode == SPI_CONTROLLER)
        {
            /*
             * IO mux needs to occur after SPI IP is enabled
             * to produce correct signal state.
             */
            initIO(handle);
        }
    }
}

/** @brief 检查 TX DMA 传输是否完成（剩余传输量为 0）。 */
static bool isDMATxDone(SPIMSPM0_Object* object)
{
    if(DL_DMA_getTransferSize(DMA, object->txDMAChannel) == 0)
    {
        return true;
    }
    else
    {
        return false;
    }
}
/*
 *  ======== enableDMA ========
 *  Enable the SPI DMA peripheral
 */
/** @brief 使能 SPI DMA 触发事件（TX 和 RX）。 */
static void enableDMA(SPIMSPM0_HWAttrs const *hwAttrs)
{
    if(!DMATxRxEnabled)
    {
        /* Enable SPI TX interrupt as a trigger for DMA */
        DL_SPI_enableDMATransmitEvent(hwAttrs->spi);
        /* Enable SPI RX interrupt as a trigger for DMA */
        DL_SPI_enableDMAReceiveEvent(hwAttrs->spi, DL_SPI_DMA_INTERRUPT_RX);
        DMATxRxEnabled = true;
    }
}

/** @brief 禁用 SPI DMA 通道和触发事件。 */
static void disableDMA(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    if(DMATxRxEnabled)
    {
        DMATxRxEnabled = false;

        DMAMSPM0_disableChannel(object->DMA_Handle, object->txDMAChannel);
        DMAMSPM0_disableChannel(object->DMA_Handle, object->rxDMAChannel);
        /* Disable the DMA trigger for TX and RX */
        DL_SPI_disableDMATransmitEvent(hwAttrs->spi);
        DL_SPI_disableDMAReceiveEvent(hwAttrs->spi,DL_SPI_DMA_INTERRUPT_RX);
    }
}

/** @brief 仅禁用 RX DMA 通道和触发事件。 */
static void disableDMARx(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    DMAMSPM0_disableChannel(object->DMA_Handle, object->rxDMAChannel);
    /* Disable the DMA trigger for RX */

    DL_SPI_disableDMAReceiveEvent(hwAttrs->spi,DL_SPI_DMA_INTERRUPT_RX);
}

/** @brief 仅禁用 TX DMA 通道和触发事件。 */
static void disableDMATx(SPI_Handle handle)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    DMAMSPM0_disableChannel(object->DMA_Handle, object->txDMAChannel);

    /* Disable the DMA trigger for TX */
    DL_SPI_disableDMATransmitEvent(hwAttrs->spi);
}
/*
 *  ======== configSPI ========
 *  Configures the peripheral settings
 */
/**
 * @brief 配置 SPI 外设参数。
 * 设置模式、位序、数据大小、帧格式、校验、片选、FIFO 阈值和时钟分频。
 *
 * @param handle SPI 句柄
 * @param freq   CPU 时钟频率（Hz）
 * @param format 帧格式值
 * @return 始终返回 true
 */
static bool configSPI(SPI_Handle handle, uint32_t freq, uint32_t format)
{
    SPIMSPM0_Object *object         = handle->object;
    SPIMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint16_t scr;
    uint32_t ratio;
    DL_SPI_Config dlCfg;

    DL_SPI_ClockConfig clockConfig =
    {
    .clockSel    = hwAttrs->clockSource,
    .divideRatio = hwAttrs->clockDivider
    };

    DL_SPI_setClockConfig(hwAttrs->spi, (DL_SPI_ClockConfig *) &clockConfig);

    if(object->mode == SPI_CONTROLLER)
    {
        dlCfg.mode = DL_SPI_MODE_CONTROLLER;
    }
    else
    {
        dlCfg.mode = DL_SPI_MODE_PERIPHERAL;
    }
    if(object->bitOrder)
    {
        dlCfg.bitOrder = DL_SPI_BIT_ORDER_LSB_FIRST;
    }
    else
    {
        dlCfg.bitOrder = DL_SPI_BIT_ORDER_MSB_FIRST;
    }
    dlCfg.dataSize = dataSize[object->dataSize - 4];
    dlCfg.frameFormat = format;

    switch(object->parity)
    {
        case SPI_PARITY_EVEN:
            dlCfg.parity = DL_SPI_PARITY_EVEN;
        break;
        case SPI_PARITY_ODD:
            dlCfg.parity = DL_SPI_PARITY_ODD;
        break;
        case SPI_PARITY_NONE:
            dlCfg.parity = DL_SPI_PARITY_NONE;
        break;
        default:
            dlCfg.parity = DL_SPI_PARITY_NONE;
        break;
    };

    switch(hwAttrs->cssel)
    {
        case SPI_CHIP_SELECT_0:
            dlCfg.chipSelectPin = DL_SPI_CHIP_SELECT_0;
        break;
        case SPI_CHIP_SELECT_1:
            dlCfg.chipSelectPin = DL_SPI_CHIP_SELECT_1;
        break;
        case SPI_CHIP_SELECT_2:
            dlCfg.chipSelectPin = DL_SPI_CHIP_SELECT_2;
        break;
        case SPI_CHIP_SELECT_3:
            dlCfg.chipSelectPin = DL_SPI_CHIP_SELECT_3;
        break;
        case SPI_CHIP_SELECT_NONE:
            dlCfg.chipSelectPin = DL_SPI_CHIP_SELECT_NONE;
        break;
        default:
            dlCfg.chipSelectPin = DL_SPI_CHIP_SELECT_NONE;
        break;
    };

    DL_SPI_init(hwAttrs->spi, (DL_SPI_Config*) &dlCfg);
    DL_SPI_setFIFOThreshold(hwAttrs->spi, hwAttrs->rxFifoThreshold, hwAttrs->txFifoThreshold);

    if(object->bitRate == 0)
    {
        object->bitRate = 500000; /* If value is not set then set to default value */
    }

    /* Calculate scr variable */
    ratio = freq / (2 * object->bitRate);
    if (ratio > 0 && ratio <= SERIAL_CLK_DIVIDER_MAX)
    {
        scr = (uint16_t)(ratio - 1);
    }
    else
    {
        scr = 0;
    }
    /* Set clock divider */
    DL_SPI_setBitRateSerialClockDivider(hwAttrs->spi, scr);

    return (true);
}
/* clang-format on */
