/*
 * Copyright (c) 2021, Texas Instruments Incorporated
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
/* MSPM0 UART驱动硬件层实现，包含中断处理、DMA传输和外设配置 */
#include <string.h>
#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/uart/UARTMSPM0.h>

static bool UARTMSP_enable(UART_Handle handle, UART_Params *params);
static void UARTMSP_disable(UART_Handle handle);
static void UARTMSP_txChar(UART_Handle handle);

#ifndef UART_NO_DMA /* This #define with remove the DMA related implementation to reduce the code size */
#include <ti/drivers/dma/DMAMSPM0.h>
__STATIC_INLINE size_t UARTMSP_getRxData(UART_Handle handle, size_t size);
static void UARTMSP_readFromRingBuf(UART_Handle handle);
static void UARTMSP_readTimeout(uintptr_t arg);
static void UARTMSP_writeTimeout(uintptr_t arg);
static volatile DMAMSPM0_Handle dmaHandle;
static DMAMSPM0_Transfer DMATransfer;
static DL_DMA_Config dlDMACfg;
#endif

/* UART支持函数表，绑定底层实现到驱动框架 */
const UARTSupport_Fxns UARTMSPSupportFxns =
{
    .enable  = UARTMSP_enable,
    .disable = UARTMSP_disable,
    .txChar  = UARTMSP_txChar,
    .read    = UART_readFullFeatured,
    .write   = UART_writeFullFeatured,
};

/**
 *  @brief  默认事件回调函数（弱符号），用户未提供时使用
 */
void __WEAK UARTMSP_eventCallback(UART_Handle handle, uint32_t event, uint32_t data, void *userArg)
{
}

/**
 *  @brief  初始化缓冲区支持，构造RX/TX环形缓冲区和信号量
 *  @param[in]  object  UART_Buffers_Object指针
 */
__STATIC_INLINE void UARTMSP_initBuffers(UART_Buffers_Object *object)
{
    /* 构造收发环形缓冲区 */
    RingBuf_construct(&object->rxBuf, object->rxBufPtr, object->rxBufSize);
    RingBuf_construct(&object->txBuf, object->txBufPtr, object->txBufSize);

    /* 创建收发信号量（初始值为1，首次pend不会阻塞） */
    object->rxSem = SemaphoreP_createBinary(1);
    object->txSem = SemaphoreP_createBinary(1);
}

/* 默认UART参数：阻塞模式，9600波特率，8N1 */
const UART_Params UART_defaultParams =
{
    UART_Mode_BLOCKING,
    UART_Mode_BLOCKING,
    NULL,
    NULL,
    NULL,
    0,
    UART_ReadReturnMode_PARTIAL,
    9600,
    UART_DataLen_8,
    UART_StopBits_1,
    UART_Parity_NONE,
    NULL,
};

/**
 *  @brief  使能UART外设，完成GPIO、时钟、波特率、FIFO和中断配置
 *  @param[in]  handle  UART句柄
 *  @param[in]  params  UART参数
 *  @return true成功，false失败
 */
static bool UARTMSP_enable(UART_Handle handle, UART_Params *params)
{
    uint32_t divratio = 0;
    uint32_t uartClk  = 0;
    uint32_t clkFreq  = 0;

    UARTMSP_HWAttrs *hwAttrs           = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject            = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(uartObject);
    UART_EventCallback_Object *eventCallbackObj = UART_eventCallbackObject(uartObject);

    /* 事件回调为空时加载默认回调并清零事件掩码 */
    if ((uartObject->eventsSupported == true) && (eventCallbackObj->eventCallback == NULL))
    {
        eventCallbackObj->eventCallback = UARTMSP_eventCallback;
        eventCallbackObj->eventMask     = 0;
    }

    /* 初始化缓冲区和信号量 */
    if (uartObject->buffersSupported == true)
    {
        UARTMSP_initBuffers(buffersObject);
    }

    /* 配置TX/RX引脚复用功能 */
    DL_GPIO_initPeripheralOutputFunction(hwAttrs->txPin, hwAttrs->txPinFunction);
    DL_GPIO_initPeripheralInputFunction(hwAttrs->rxPin, hwAttrs->rxPinFunction);

    /* 复位UART外设并使能电源 */
    DL_UART_reset(hwAttrs->regs);
    DL_UART_enablePower(hwAttrs->regs);

    /* 配置时钟源和分频 */
    DL_UART_ClockConfig clockConfig =
    {
        .clockSel    = hwAttrs->clockSource,
        .divideRatio = hwAttrs->clockDivider,
    };
    DL_UART_setClockConfig(hwAttrs->regs, &clockConfig);

    /* 根据时钟源确定时钟频率 */
    switch (clockConfig.clockSel)
    {
        case DL_UART_CLOCK_BUSCLK:
            uartClk = 32000000; /* 32Mhz */
            break;
        case DL_UART_CLOCK_MFCLK:
            uartClk = 4000000; /* 4Mhz */
            break;
        case DL_UART_CLOCK_LFCLK:
            uartClk = 32000; /* 32Khz */
            break;
    }

    /* 计算实际时钟频率（向上取整） */
    divratio = clockConfig.divideRatio + 1;
    clkFreq = (uint32_t)((uartClk + (divratio - 1)) / divratio);

    /* 初始化UART：模式、方向、流控、校验、数据位、停止位 */
    DL_UART_Config config =
    {
        .mode        = hwAttrs->mode,
        .direction   = hwAttrs->direction,
        .flowControl = hwAttrs->flowControl,
        .parity      = (DL_UART_PARITY) params->parityType,
        .wordLength  = (DL_UART_WORD_LENGTH) params->dataLength,
        .stopBits    = (DL_UART_STOP_BITS) params->stopBits,
    };
    DL_UART_init(hwAttrs->regs, &config);
    /* 配置波特率 */
    DL_UART_configBaudRate(hwAttrs->regs, clkFreq, params->baudRate);
    /* 使能FIFO并设置阈值 */
    DL_UART_enableFIFOs(hwAttrs->regs);
    DL_UART_setRXFIFOThreshold(hwAttrs->regs, hwAttrs->rxIntFifoThr);
    DL_UART_setTXFIFOThreshold(hwAttrs->regs, hwAttrs->txIntFifoThr);
#ifndef UART_NO_DMA
    if ((params->readMode == UART_Mode_CALLBACK || params->writeMode == UART_Mode_CALLBACK) && (uartObject->dmaSupported == true))
    {
        /* This function will return pointer to array of DMA handles
         * which depends on the number of DMA channels
         * */
        uartObject->DMA_Handle = DMA_Init(&DMATransfer, &dlDMACfg, uartObject->noOfDMAChannels);

        if (params->readMode == UART_Mode_CALLBACK)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_DMA_DONE_RX);
        }

        if (params->writeMode == UART_Mode_CALLBACK)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_DMA_DONE_TX | DL_UART_INTERRUPT_EOT_DONE);
        }
    }
    if (params->readMode != UART_Mode_CALLBACK)
    {
        DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_RX);
    }
#else
    DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_RX);
#endif
    /* 使能NVIC中断和UART外设 */
    HwiP_enableInterrupt(hwAttrs->irq);
    DL_UART_enable(hwAttrs->regs);

    return true;
}

/**
 *  @brief  禁用UART外设，关闭中断、清除状态、删除信号量
 *  @param[in]  handle  UART句柄
 */
static void UARTMSP_disable(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject   = UART_buffersObject(uartObject);


    HwiP_disableInterrupt(hwAttrs->irq);
    DL_UART_disableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_RX |
                                                DL_UART_INTERRUPT_TX |
                                                DL_UART_INTERRUPT_EOT_DONE);
    DL_UART_clearInterruptStatus(hwAttrs->regs, 0xFFFF);
    DL_UART_disable(hwAttrs->regs);
    uartObject->readInUse  = false;
    uartObject->writeInUse = false;
    SemaphoreP_delete(buffersObject->rxSem);
    SemaphoreP_delete(buffersObject->txSem);
#ifndef UART_NO_DMA
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);
    if ((callbackObject->readCallback != NULL) || (callbackObject->writeCallback != NULL))
    {
        DMAMSPM0_close(DMA, DMATransfer.dmaChannel);
    }
#endif
    return;
}

/**
 *  @brief  触发发送：有DMA回调时走DMA，否则使能TX中断
 *  @param[in]  handle  UART句柄
 */
static void UARTMSP_txChar(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
#ifndef UART_NO_DMA
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);

    if (callbackObject->writeCallback != NULL)
    {
        /* if the callback is set then call tx with dma */
        UARTMSP_dmaTx(handle);
    }
    else
    {
        /* enable transmit interrupts */
        DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_TX);
    }
#else
    /* enable transmit interrupts */
        DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_TX);
#endif
}
#ifndef UART_NO_DMA
/**
 *  @brief  启动DMA发送，从环形缓冲区读取数据通过DMA发送到UART TX
 *  @param[in]  handle  UART句柄
 */
void UARTMSP_dmaTx(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject   = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);
    unsigned char *srcAddr;

    if (callbackObject->txSize > 0)
    {
        /* DMA TX already in progress */
        return;
    }

    callbackObject->txSize = RingBuf_getPointer(&buffersObject->txBuf, &srcAddr);

    if (callbackObject->txSize > 0)
    {
        DL_UART_disableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_EOT_DONE);
        DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_DMA_DONE_TX);
        dlDMACfg.trigger              = DMATransfer.txTrigger;
        dlDMACfg.triggerType          = DMATransfer.txTriggerType;
        dlDMACfg.destIncrement        = DL_DMA_ADDR_UNCHANGED;
        dlDMACfg.srcIncrement         = DL_DMA_ADDR_INCREMENT;
        DMATransfer.dmaTransferSource = (uint8_t *) srcAddr;
        DMATransfer.dmaTransferDestination = (uint32_t *) (&hwAttrs->regs->TXDATA);
        DMATransfer.noOfData     = callbackObject->txSize;
        uartObject->txDmaChannel = DMATransfer.dmaChannel;
        /* set DMA transfer */
        DMAMSPM0_setupTransfer(&DMATransfer, &dlDMACfg);
        /* enable the transmit interrupt */
        DL_UART_enableDMATransmitEvent(hwAttrs->regs);
    }
}
/**
 *  @brief  启动DMA接收，配置DMA从UART RX读取数据到环形缓冲区或用户缓冲区
 *  @param[in]  handle    UART句柄
 *  @param[in]  copyfifo  true时先复制FIFO中已有数据
 */
void UARTMSP_dmaRx(UART_Handle handle, bool copyfifo)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject   = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);

    unsigned char *dstAddr;

    if (uartObject->buffersSupported == true)
    {
        if (callbackObject->rxSize > 0)
        {
            /* DMA RX already in progress */
            return;
        }

        if (copyfifo)
        {
            /* Copy as much out of the RX FIFO as as we can, this helps us keep up at high data rates. */
            if (UARTMSP_getRxData(handle, RingBuf_space(&buffersObject->rxBuf)) > 0)
            {
                SemaphoreP_post(buffersObject->rxSem);
            }
        }
        /* get the remaining space in the buffer for the DMA transaction */
        callbackObject->rxSize = RingBuf_putPointer(&buffersObject->rxBuf, &dstAddr);

        if (callbackObject->rxSize > 0)
        {
            uartObject->overrunActive = false;
            if ((uartObject->readMode == UART_Mode_BLOCKING || uartObject->readMode == UART_Mode_CALLBACK) &&
                                   callbackObject->rxSize > (buffersObject->rxBufSize / 2))
            {
                /* In blocking read mode, there are only two ways that the read()
                 * can be unblocked:
                 *   1) receive timeout interrupt
                 *   2) DMA finished
                 *
                 * In blocking mode, we make sure the DMA transfer is no longer
                 * than half the RX buffer size in order to leave some space for
                 * the driver to keep filling while the application moves to
                 * unload the buffer through one or more read() calls.
                 */
                callbackObject->rxSize = buffersObject->rxBufSize / 2;
                if (callbackObject->rxSize < 1)
                {
                    callbackObject->rxSize = 1; /* keep the size at least 1byte */
                }
            }
        }
    }
    else
    {
        /* user supplied rx buffer is used to receive data directly */
        dstAddr = (unsigned char *) callbackObject->readBuf;
    }
    DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_DMA_DONE_RX);
    dlDMACfg.trigger                   = DMATransfer.rxTrigger;
    dlDMACfg.triggerType               = DMATransfer.rxTriggerType;
    dlDMACfg.destIncrement             = DL_DMA_ADDR_INCREMENT;
    dlDMACfg.srcIncrement              = DL_DMA_ADDR_UNCHANGED;
    DMATransfer.dmaTransferSource      = (uint8_t *) (&hwAttrs->regs->RXDATA);
    DMATransfer.dmaTransferDestination = (uint8_t *) dstAddr;
    DMATransfer.noOfData               = callbackObject->rxSize;
    uartObject->rxDmaChannel           = DMATransfer.dmaChannel;
    /* set DMA transfer */
    DMAMSPM0_setupTransfer(&DMATransfer, &dlDMACfg);
    /* Use UART Rx interrupt as a DMA Trigger */
    DL_UART_enableDMAReceiveEvent(hwAttrs->regs, DL_UART_DMA_INTERRUPT_RX);
}
#else
/**
 *  @brief  无DMA模式下的TX回调，从回调缓冲区逐字节发送，应从ISR调用
 *  @param[in]  handle  UART句柄
 */
void UARTMSP_TxCallback(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs           = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject            = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObj  = UART_callbackObject(uartObject);

    do
    {
        uint8_t txData;
        txData = *(callbackObj->writeBuf + callbackObj->bytesWritten);
        if (callbackObj->writeSize == callbackObj->bytesWritten)
        {
            /* no more data left to transmit, disable TX interrupt, enable
                * end of transmission interrupt for low power support.
                */
            DL_UART_disableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_TX);
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_EOT_DONE);

            break;
        }
        /* pulled a byte of data out, prime the transmitter */
        DL_UART_transmitData(hwAttrs->regs, txData);
        callbackObj->bytesWritten++;

    } while (DL_UART_isTXFIFOFull(hwAttrs->regs) == false);

}
#endif
/**
 *  @brief  从UART RX FIFO读取数据到环形缓冲区（需在中断禁用下调用）
 *  @param[in]  handle  UART句柄
 *  @param[in]  size    最大读取字节数
 *  @return 实际读取的字节数
 */
__STATIC_INLINE size_t UARTMSP_getRxData(UART_Handle handle, size_t size)
{
    UARTMSP_HWAttrs *hwAttrs           = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject            = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(uartObject);

    size_t consumed = 0;
    uint8_t data;

    /* 从FIFO逐字节读取到环形缓冲区 */
    while (!(DL_UART_isRXFIFOEmpty(hwAttrs->regs)) && size)
    {
        data = DL_UART_receiveData(hwAttrs->regs);
        RingBuf_put(&buffersObject->rxBuf, data);
        ++consumed;
        --size;
    }
    return (consumed);
}

/**
 *  @brief  UART中断处理主函数，处理RX/TX/DMA/错误/超时等所有中断
 *  @param[in]  handle  UART句柄
 */
void UARTMSP_interruptHandler(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs           = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject            = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObj  = UART_callbackObject(uartObject);
    UART_EventCallback_Object *eventCallbackObj = UART_eventCallbackObject(uartObject);

    int space;
    static unsigned char *dstAddr;
    uint16_t count = 0;
    uint32_t event;

    /* 获取所有已使能的中断状态 */
    uint32_t status = DL_UART_getEnabledInterruptStatus(hwAttrs->regs, 0x1FFFF);
    bool rxPost = false;
    bool txPost = false;

    /* 处理接收错误事件（溢出、中断、校验、帧错误） */
    if ((status & (UART_RX_ERRS)) && uartObject->eventsSupported)
    {
        uartObject->overrunActive = false;
        uartObject->overrunCount  = 0;
        if (status & DL_UART_INTERRUPT_OVERRUN_ERROR)
        {
            /* 溢出错误：尽可能读取已有数据 */
            UARTMSP_getRxData(handle, RingBuf_space(&buffersObject->rxBuf));
            /* 丢弃剩余数据以清除溢出标志 */
            while (!DL_UART_isRXFIFOEmpty(hwAttrs->regs))
            {
                volatile uint8_t data = DL_UART_receiveData(hwAttrs->regs);
                (void) data;
            }
            ++uartObject->overrunCount;
            if (uartObject->overrunActive == false)
            {
                uartObject->overrunActive = true;
            }
        }

        event = status & (UART_RX_ERRS);
        if (event)
        {
            eventCallbackObj->eventCallback(handle, event, uartObject->overrunCount, callbackObj->userArg);
        }
    }
#ifndef UART_NO_DMA
    /* 处理DMA接收完成和接收超时中断 */
    if ((status & DL_UART_INTERRUPT_DMA_DONE_RX) || (status & DL_UART_INTERRUPT_RX_TIMEOUT_ERROR))
    {
        /* 停止DMA接收 */
        UARTMSP_dmaStopRx(handle);
        /* 接收超时时需先清空FIFO再重启DMA */
        if (status & DL_UART_INTERRUPT_RX_TIMEOUT_ERROR)
        {
            UARTMSP_dmaRx(handle, true);
        }

        /* 回调模式：从环形缓冲区提取数据并判断是否触发回调 */
        if ((uartObject->readMode == UART_Mode_CALLBACK) && callbackObj->readCount)
        {
            UARTMSP_readFromRingBuf(handle);

            /* 全部读完或部分模式收到超时时触发回调 */
            if (((uartObject->returnMode == UART_ReadReturnMode_PARTIAL) && (status & DL_UART_INTERRUPT_RX_TIMEOUT_ERROR)) ||
                                             (callbackObj->readCount == 0))
            {
                /* 重置readCount避免回调被重复调用 */
                uartObject->readInUse  = false;
                callbackObj->readCount = 0;
                callbackObj->readCallback(handle,
                    (void *) callbackObj->readBuf, callbackObj->bytesRead,
                    callbackObj->userArg, UART_STATUS_SUCCESS);
                /* 释放信号量 */
                rxPost = true;
            }
        }
    }
    if (status & DL_UART_INTERRUPT_DMA_DONE_TX)
    {
        UARTMSP_dmaStopTx(handle);
        if ((uartObject->writeMode == UART_Mode_CALLBACK) && callbackObj->writeCount)
        {
            space = RingBuf_putPointer(&buffersObject->txBuf, &dstAddr);
            if (space > callbackObj->writeCount)
            {
                space = callbackObj->writeCount;
            }
            memcpy(dstAddr, callbackObj->writeBuf + callbackObj->bytesWritten, space);

            /* Update the ring buffer state with the number of bytes copied */
            RingBuf_putAdvance(&buffersObject->txBuf, space);

            callbackObj->bytesWritten += space;
            callbackObj->writeCount -= space;

            callbackObj->txSize = 0;
            /* Start dma in case txSize was 0 */
            UARTMSP_dmaTx(handle);

            if ((callbackObj->writeCount == 0) && uartObject->writeInUse)
            {
                uartObject->writeInUse = false;
                callbackObj->writeCallback(handle,
                    (void *) callbackObj->writeBuf, callbackObj->bytesWritten,
                    callbackObj->userArg, UART_STATUS_SUCCESS);

                /*  No more data pending in the TX buffer, wait for it to finish
                 *  shifting out of the transmit shift register.
                 */
                DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_EOT_DONE);
            }
            return;
        }

        if (callbackObj->writeCount == 0)
        {
            /*
             *  No more data pending in the TX buffer, wait for it to finish
             *  shifting out of the transmit shift register.
             */
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_EOT_DONE);
        }

        /* Since the DMA has done with tx, clear the interrupts */
        DL_UART_disableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_DMA_DONE_TX);
    }
#endif
    /* 处理RX中断：从FIFO读取数据到环形缓冲区 */
    if (status & DL_UART_INTERRUPT_RX)
    {
        while (DL_UART_isRXFIFOEmpty(hwAttrs->regs) == false)
        {
            /* 从硬件FIFO读取数据 */
            volatile uint8_t rxData = DL_UART_receiveData(hwAttrs->regs);

            /* 推入环形缓冲区，溢出时丢弃 */
            if (RingBuf_put(&buffersObject->rxBuf, rxData) == -1)
            {
                /* RX缓冲区已满，溢出 */
            }
#ifdef UART_NO_DMA /* For reduced code only, no DMA*/
            if (callbackObj->readCallback != NULL)
            {
                /* get the data from ring buffer to the application provided buffer */
                count += RingBuf_getn(&buffersObject->rxBuf, callbackObj->readBuf + count, callbackObj->readSize - count);
                if(callbackObj->readSize == count)
                {
                    count = 0; /* reset the counter */
                    uartObject->readInUse = false;
                    callbackObj->readCallback(handle, (void*)callbackObj->readBuf, callbackObj->readSize,callbackObj->userArg, UART_STATUS_SUCCESS);
                }
            }
#endif
            rxPost = true; /* release the semaphore */
        }
    }
    /* 处理TX中断：从环形缓冲区取数据发送 */
    if (status & DL_UART_INTERRUPT_TX)
    {
#ifdef UART_NO_DMA /* For reduced code only, no DMA*/
        if (callbackObj->writeCallback != NULL)
        {
            UARTMSP_TxCallback(handle);
        }
        else
        {
#endif
        /* because we had an active interrupt flag, we should always have
         * room for at least one byte of transmit data
         */
        do
        {
            uint8_t txData;
            if (RingBuf_get(&buffersObject->txBuf, &txData) == -1)
            {
                /* no more data left to transmit, disable TX interrupt, enable
                * end of transmission interrupt for low power support.
                */
                DL_UART_disableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_TX);
                DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_EOT_DONE);
                break;
            }
            /* pulled a byte of data out, prime the transmitter */
            DL_UART_transmitData(hwAttrs->regs, txData);

        } while (DL_UART_isTXFIFOFull(hwAttrs->regs) == false);
#ifdef UART_NO_DMA /* For reduced code only, no DMA*/
        }
#endif
    }

    /* 处理传输完成中断（EOT） */
    if (status & DL_UART_INTERRUPT_EOT_DONE)
    {
        /* 传输完成，禁用中断 */
        DL_UART_disableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_EOT_DONE);
        txPost = true;
#ifndef UART_NO_DMA
        if (callbackObj->writeCallback != NULL)
        {
            if (uartObject->writeInUse != false)
            {
                uartObject->writeInUse = false;
                callbackObj->writeCallback(handle,
                    (uint8_t *) callbackObj->writeBuf, callbackObj->writeSize,
                    callbackObj->userArg, 0);
            }
        }
#endif
    }

    /* 释放接收信号量 */
    if (rxPost)
    {
        SemaphoreP_post(buffersObject->rxSem);
    }
    /* 释放发送信号量 */
    if (txPost)
    {
        SemaphoreP_post(buffersObject->txSem);
    }
}

/**
 *  @brief  根据事件掩码使能接收、超时和错误中断
 *  @param[in]  handle  UART句柄
 */
void UARTMSP_enableInts(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject  = UART_Obj_Ptr(handle);
    UART_EventCallback_Object *eventCallbackObj = UART_eventCallbackObject(uartObject);

    if (eventCallbackObj->eventCallback)
    {
        if (eventCallbackObj->eventMask & DL_UART_INTERRUPT_OVERRUN_ERROR)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_OVERRUN_ERROR);
        }
        if (eventCallbackObj->eventMask & DL_UART_INTERRUPT_BREAK_ERROR)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_BREAK_ERROR);
        }
        if (eventCallbackObj->eventMask & DL_UART_INTERRUPT_PARITY_ERROR)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_PARITY_ERROR);
        }
        if (eventCallbackObj->eventMask & DL_UART_INTERRUPT_FRAMING_ERROR)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_FRAMING_ERROR);
        }
        if (eventCallbackObj->eventMask & DL_UART_INTERRUPT_RX_TIMEOUT_ERROR)
        {
            DL_UART_enableInterrupt(hwAttrs->regs, DL_UART_INTERRUPT_RX_TIMEOUT_ERROR);
        }
    }
}
#ifndef UART_NO_DMA
/**
 *  @brief  停止DMA接收，禁用通道并更新环形缓冲区状态（需在中断禁用下调用）
 *  @param[in]  handle  UART句柄
 */
void UARTMSP_dmaStopRx(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Buffers_Object *uartBufferObj   = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);

    uint32_t bytesRemaining;
    uint32_t rxCount;
    int advanced;

    if (callbackObject->rxSize > 0)
    {
        /* 禁用DMA通道和触发源 */
        DMAMSPM0_disableChannel(uartObject->DMA_Handle, uartObject->rxDmaChannel);
        DL_UART_disableDMAReceiveEvent(hwAttrs->regs, DL_UART_DMA_INTERRUPT_RX);
        DL_UART_clearInterruptStatus(hwAttrs->regs, DL_UART_DMA_DONE_INTERRUPT_RX);
        /* 计算已接收字节数并更新环形缓冲区 */
        bytesRemaining = DMAMSPM0_getCurrTransferSize(uartObject->rxDmaChannel);
        rxCount  = callbackObject->rxSize - bytesRemaining;
        advanced = RingBuf_putAdvance(&uartBufferObj->rxBuf, rxCount);

        callbackObject->rxSize = 0;
    }
    (void) advanced;
}
/**
 *  @brief  停止DMA发送，禁用通道并更新环形缓冲区状态（需在中断禁用下调用）
 *  @param[in]  handle  UART句柄
 *  @return 剩余待发送字节数
 */
uint32_t UARTMSP_dmaStopTx(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Buffers_Object *uartBufferObj   = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);

    uint32_t bytesRemaining = 0;
    uint32_t txCount;
    int consumed;

    if (callbackObject->txSize > 0)
    {
        /* 禁用DMA通道和触发源 */
        DMAMSPM0_disableChannel(uartObject->DMA_Handle, uartObject->txDmaChannel);
        DL_UART_disableDMATransmitEvent(hwAttrs->regs);
        DL_UART_clearInterruptStatus(hwAttrs->regs, DL_UART_DMA_DONE_INTERRUPT_TX);
        /* 计算已发送字节数并消费环形缓冲区数据 */
        bytesRemaining = DMAMSPM0_getCurrTransferSize(uartObject->txDmaChannel);
        txCount  = callbackObject->txSize - bytesRemaining;
        consumed = RingBuf_getConsume(&uartBufferObj->txBuf, txCount);

        callbackObject->txSize = 0;
    }

    (void) consumed;

    return (bytesRemaining);
}
/**
 *  @brief  从环形缓冲区提取数据到用户缓冲区，并重启DMA接收
 *  @param[in]  handle  UART句柄
 */
static void UARTMSP_readFromRingBuf(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs             = UART_HWAttrs_Ptr(handle);
    UART_Object *uartObject              = UART_Obj_Ptr(handle);
    UART_Buffers_Object *uartBufferObj   = UART_buffersObject(uartObject);
    UART_Callback_Object *callbackObject = UART_callbackObject(uartObject);

    unsigned char *srcAddr;
    int available;

    /* 从环形缓冲区循环提取数据到用户缓冲区 */
    do
    {
        available = RingBuf_getPointer(&uartBufferObj->rxBuf, &srcAddr);

        if (available > callbackObject->readCount)
        {
            available = callbackObject->readCount;
        }

        memcpy(callbackObject->readBuf + callbackObject->bytesRead, srcAddr, available);
        RingBuf_getConsume(&uartBufferObj->rxBuf, available);
        callbackObject->readCount -= available;
        callbackObject->bytesRead += available;

    } while (available && callbackObject->readCount);

    /* Restart the DMA since we cleared space in the ring buffer */
    UARTMSP_dmaStopRx(handle);
    /* Do not manually copy FIFO at this point, only set up the DMA transaction,
     * because the uart object might become unaware of these manually copied bytes,
     * and we need to leave data in the FIFO for a receive-timeout to occur
     */
    if (callbackObject->readCount > 0) UARTMSP_dmaRx(handle, false);
}

/**
 *  @brief  读超时回调函数，由时钟模块调用，唤醒阻塞的读任务
 *  @param[in]  arg  UART句柄
 */
static void UARTMSP_readTimeout(uintptr_t arg)
{
    UART_Handle handle                 = (UART_Handle) arg;
    UART_Object *uartObject            = UART_Obj_Ptr(handle);
    UART_Buffers_Object *uartBufferObj = UART_buffersObject(uartObject);

    uartObject->readTimedOut = true;
    SemaphoreP_post(uartBufferObj->rxSem);
}

/**
 *  @brief  写超时回调函数，由时钟模块调用，唤醒阻塞的写任务
 *  @param[in]  arg  UART句柄
 */
static void UARTMSP_writeTimeout(uintptr_t arg)
{
    UART_Handle handle                 = (UART_Handle) arg;
    UART_Object *uartObject            = UART_Obj_Ptr(handle);
    UART_Buffers_Object *uartBufferObj = UART_buffersObject(uartObject);

    uartObject->writeTimedOut = true;
    SemaphoreP_post(uartBufferObj->txSem);
    /* Post one extra time if in blocking mode, as the write-function is
       pending on this semaphore until EOT interrupt is received */
    if (uartObject->writeMode == UART_Mode_BLOCKING)
    {
        SemaphoreP_post(uartBufferObj->txSem);
    }
}
#endif
/* clang-format on */
