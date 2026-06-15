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
/* UART驱动核心实现文件，提供阻塞/回调/非阻塞模式的读写操作 */
#include <string.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/devices/DeviceFamily.h>
#include <ti/drivers/uart/UARTMSPM0.h>

extern const uint_least8_t UART_count;
extern const UART_Params UART_defaultParams;

static int_fast16_t UART_writeBufferedMode(UART_Handle handle,
    const void *buf, size_t size, size_t *bytesWritten, uint8_t mode);
static int_fast16_t UART_readBufferedMode(UART_Handle handle, void *buf,
    size_t size, size_t *bytesRead, uint8_t mode);


/**
 *  @brief  缓冲模式读取，内部辅助函数
 *
 *  @param[in]  handle    UART句柄
 *  @param[in]  buf       接收缓冲区
 *  @param[in]  size      要读取的字节数
 *  @param[out] bytesRead 实际读取字节数
 *  @param[in]  mode      工作模式（阻塞/非阻塞）
 *  @return 状态码
 */
static int_fast16_t UART_readBufferedMode(UART_Handle handle, void *buf,
    size_t size, size_t *bytesRead, uint8_t mode)
{
    UART_Object *uartObject                = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(uartObject);

    uint8_t *data                      = (uint8_t *) (buf);
    uint_fast16_t count                = 0;
    int_fast16_t result                = UART_STATUS_SUCCESS;
    uintptr_t key;

    /* 清除取消标志 */
    key                = HwiP_disable();
    uartObject->readCancel = false;
    HwiP_restore(key);

    while (size - count)
    {
        /* 等待信号量，ISR接收字符后会释放信号量，任务在此休眠 */
        SemaphoreP_pend(buffersObject->rxSem, SemaphoreP_WAIT_FOREVER);

        if (uartObject->readCancel)
        {
            result = UART_STATUS_ECANCELLED;
            break;
        }
        /* 从环形缓冲区读取数据 */
        count += RingBuf_getn(&buffersObject->rxBuf, ((uint8_t *) data) + count, size - count);

        if (mode == UART_Mode_NONBLOCKING)
        {
            /* 非阻塞模式下无数据则返回EAGAIN */
            if (count == 0)
            {
                result = UART_STATUS_EAGAIN;
            }
            break;
        }
    }
    if (bytesRead)
    {
        *bytesRead = count;
    }
    return result;
}

/**
 *  @brief  缓冲模式写入，内部辅助函数
 *
 *  @param[in]  handle        UART句柄
 *  @param[in]  buf           待发送数据缓冲区
 *  @param[in]  size          要发送的字节数
 *  @param[out] bytesWritten  实际发送字节数
 *  @param[in]  mode          工作模式（阻塞/非阻塞）
 *  @return 状态码
 */
static int_fast16_t UART_writeBufferedMode(UART_Handle handle,
    const void *buf, size_t size, size_t *bytesWritten, uint8_t mode)
{
    UART_Object *uartObject                = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(uartObject);
    const uint8_t *data                = (const uint8_t *) (buf);
    uint_fast16_t count                = 0;
    int_fast16_t result                = UART_STATUS_SUCCESS;
    uintptr_t key;

    /* 清除取消标志 */
    key                 = HwiP_disable();
    uartObject->writeCancel = false;
    HwiP_restore(key);

    while (size - count)
    {
        /* 等待信号量，表示前一次传输完成，可以再次加载缓冲区 */
        SemaphoreP_pend(buffersObject->txSem, SemaphoreP_WAIT_FOREVER );

        /* 将数据写入环形缓冲区 */
        count += RingBuf_putn(&buffersObject->txBuf, ((uint8_t *) data) + count, size - count);

        if (mode == UART_Mode_NONBLOCKING)
        {
            if (count == 0)
            {
                result = UART_STATUS_EAGAIN;
            }
            break;
        }
        /* 触发发送 */
        uartObject->supportFxns->txChar(handle);

        if (uartObject->writeCancel)
        {
            result = UART_STATUS_ECANCELLED;
            break;
        }
    }
    if (bytesWritten)
    {
        *bytesWritten = count;
    }
    return result;
}

/**
 *  @brief  缓冲模式读取接口，根据当前读模式调用内部缓冲读取
 *
 *  @param[in]  handle    UART句柄
 *  @param[in]  buf       接收缓冲区
 *  @param[in]  size      要读取的字节数
 *  @param[out] bytesRead 实际读取字节数
 *  @return 状态码
 */
int_fast16_t UART_readBuffered(UART_Handle handle, void *buf, size_t size, size_t *bytesRead)
{
    UART_Object *object = UART_Obj_Ptr(handle);

    return UART_readBufferedMode(handle, buf, size, bytesRead, object->readMode);
}

/**
 *  @brief  缓冲模式写入接口，根据当前写模式调用内部缓冲写入
 *
 *  @param[in]  handle        UART句柄
 *  @param[in]  buf           待发送数据缓冲区
 *  @param[in]  size          要发送的字节数
 *  @param[out] bytesWritten  实际发送字节数
 *  @return 状态码
 */
int_fast16_t UART_writeBuffered(UART_Handle handle, const void *buf, size_t size, size_t *bytesWritten)
{
    UART_Object *object = UART_Obj_Ptr(handle);

    return UART_writeBufferedMode(handle, buf, size, bytesWritten, object->writeMode);
}

/**
 *  @brief  回调模式读取，设置DMA接收并立即返回
 *
 *  @param[in]  handle    UART句柄
 *  @param[in]  buf       接收缓冲区
 *  @param[in]  size      要读取的字节数
 *  @param[out] bytesRead 实际读取字节数
 *  @return 状态码
 */
int_fast16_t UART_readCallback(UART_Handle handle, void *buf, size_t size, size_t *bytesRead)
{
    UART_Object *object                  = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject   = UART_buffersObject(object);
    UART_Callback_Object *callbackObj = UART_callbackObject(object);
    uintptr_t key;

    /* 等待前一次读操作完成 */
    SemaphoreP_pend(buffersObject->rxSem, SemaphoreP_WAIT_FOREVER);

    key = HwiP_disable();
    if (object->readInUse)
    {
        HwiP_restore(key);
        return UART_STATUS_EINUSE;
    }

    /* 初始化回调读参数 */
    object->readInUse         = true;
    callbackObj->bytesRead = 0;
    callbackObj->readBuf   = buf;
    callbackObj->readSize  = size;
    callbackObj->readCount = size;  /* 剩余待读字节数 */
    callbackObj->bytesRead = 0;
    callbackObj->rxStatus  = 0;  /* 清除接收错误 */

    /* 使能接收相关中断 */
    UARTMSP_enableInts(handle);
#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G1X0X_G3X0X)||(DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L122X_L222X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX51X) || (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L111X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G352X))
    /* 启动DMA接收 */
    UARTMSP_dmaRx(handle, false);
#endif
    HwiP_restore(key);

    return UART_STATUS_SUCCESS;
}

/**
 *  @brief  回调模式写入，将数据复制到环形缓冲区后触发发送
 *
 *  @param[in]  handle        UART句柄
 *  @param[in]  buf           待发送数据缓冲区
 *  @param[in]  size          要发送的字节数
 *  @param[out] bytesWritten  实际发送字节数
 *  @return 状态码
 */
int_fast16_t UART_writeCallback(UART_Handle handle, const void *buf, size_t size, size_t *bytesWritten)
{
    UART_Object *object                  = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject   = UART_buffersObject(object);
    UART_Callback_Object *callbackObj = UART_callbackObject(object);
    uintptr_t key;
    int space;
    static unsigned char *dstAddr;

    /* 等待TX信号量可用 */
    SemaphoreP_pend(buffersObject->txSem, SemaphoreP_WAIT_FOREVER);

    key = HwiP_disable();

    if (object->writeInUse)
    {
        HwiP_restore(key);
        return UART_STATUS_EINUSE;
    }
    /* 初始化回调写参数 */
    object->writeInUse           = true;
    callbackObj->bytesWritten = 0;
    callbackObj->writeBuf     = buf;
    callbackObj->writeSize    = size;
    callbackObj->writeCount   = size;
    callbackObj->txStatus     = 0;

    /* 尽可能多地将数据复制到环形缓冲区 */
    do {
        /* 获取环形缓冲区可写入的地址和空间大小 */
        space = RingBuf_putPointer(&buffersObject->txBuf, &dstAddr);

        key = HwiP_disable();
        if (space > callbackObj->writeCount)
        {
            space = callbackObj->writeCount;
        }
        /* 复制数据到环形缓冲区 */
        memcpy(dstAddr, (unsigned char *) callbackObj->writeBuf + callbackObj->bytesWritten, space);

        /* 更新环形缓冲区写指针 */
        RingBuf_putAdvance(&buffersObject->txBuf, space);

        callbackObj->writeCount -= space;
        callbackObj->bytesWritten += space;

        HwiP_restore(key);
    } while ((space > 0) && (callbackObj->writeCount > 0));

    /* 触发发送 */
    object->supportFxns->txChar(handle);
    HwiP_restore(key);

    return UART_STATUS_SUCCESS;
}

/**
 *  @brief  全功能读取接口，根据模式分发到缓冲或回调读取
 *
 *  @param[in]  handle    UART句柄
 *  @param[in]  buf       接收缓冲区
 *  @param[in]  size      要读取的字节数
 *  @param[out] bytesRead 实际读取字节数
 *  @return 状态码
 */
int_fast16_t UART_readFullFeatured(UART_Handle handle, void *buf, size_t size, size_t *bytesRead)
{
    UART_Object *object = UART_Obj_Ptr(handle);

    /* 非回调模式使用缓冲读取 */
    if (object->buffersSupported && object->readMode != UART_Mode_CALLBACK)
    {
        return UART_readBuffered(handle, buf, size, bytesRead);
    }
    /* 回调模式使用回调读取 */
    if (object->callbacksSupported && object->readMode == UART_Mode_CALLBACK)
    {
        return UART_readCallback(handle, buf, size, bytesRead);
    }
    return UART_STATUS_EINVALID;
}

/**
 *  @brief  全功能写入接口，根据模式分发到缓冲或回调写入
 *
 *  @param[in]  handle        UART句柄
 *  @param[in]  buf           待发送数据缓冲区
 *  @param[in]  size          要发送的字节数
 *  @param[out] bytesWritten  实际发送字节数
 *  @return 状态码
 */
int_fast16_t UART_writeFullFeatured(UART_Handle handle, const void *buf, size_t size, size_t *bytesWritten)
{
    UART_Object *object = UART_Obj_Ptr(handle);

    /* 非回调模式使用缓冲写入 */
    if (object->buffersSupported && object->writeMode != UART_Mode_CALLBACK)
    {
        return UART_writeBuffered(handle, buf, size, bytesWritten);
    }
    /* 回调模式使用回调写入 */
    if (object->callbacksSupported && object->writeMode == UART_Mode_CALLBACK)
    {
        return UART_writeCallback(handle, buf, size, bytesWritten);
    }
    return UART_STATUS_EINVALID;
}

/**
 *  @brief  将UART_Params结构体初始化为默认值
 *  @param[out] params  UART_Params指针
 */
void UART_Params_init(UART_Params *params)
{
    *params = UART_defaultParams;
}

/**
 *  @brief  打开UART外设并初始化
 *  @param[in]  index   UART_config表中的索引
 *  @param[in]  params  UART_Params指针，为NULL时使用默认参数
 *  @return UART句柄，失败返回NULL
 */
UART_Handle UART_open(uint_least8_t index, UART_Params *params)
{
    /* 回调模式下必须提供回调函数 */
    if (((params->readMode == UART_Mode_CALLBACK) && (params->readCallback == NULL)) ||
        ((params->writeMode == UART_Mode_CALLBACK) && (params->writeCallback == NULL)))
    {
        return (NULL);
    }

    if (index < UART_count)
    {
        UART_Handle handle = (UART_Handle)&UART_config[index];
        UART_Object *object = handle->object;
        uintptr_t key       = HwiP_disable();

        if (!object->inUse)
        {
            /* 标记为已打开 */
            object->inUse = true;
            HwiP_restore(key);

            if (object->supportFxns->enable(handle, params))
            {
                /* 使能成功，保存配置 */
                object->returnMode = params->readReturnMode;
                object->readMode   = params->readMode;
                object->writeMode  = params->writeMode;

                if (object->callbacksSupported)  /* 支持回调时设置回调函数 */
                {
                    UART_Callback_Object *callbackObject = UART_callbackObject(object);
                    if(params->readCallback != NULL)
                    {
                        callbackObject->readCallback  = params->readCallback;
                    }
                    if(params->writeCallback != NULL)
                    {
                        callbackObject->writeCallback = params->writeCallback;
                    }
                }
                return handle;
            }
            /* 使能失败，清除使用标志 */
            key           = HwiP_disable();
            object->inUse = false;
            HwiP_restore(key);
        }
        else
        {
            /* 已被打开 */
            HwiP_restore(key);
        }
    }
    return NULL;
}

/**
 *  @brief  关闭UART外设，释放资源
 *  @param[in]  handle  UART句柄
 */
void UART_close(UART_Handle handle)
{
    UART_Object *object = UART_Obj_Ptr(handle);
    uintptr_t key;
    /* 取消正在进行的读写操作 */
    UART_readCancel(handle);
    key = HwiP_disable();

    object->inUse = false;
    HwiP_restore(key);

    /* 禁用硬件 */
    object->supportFxns->disable(handle);
    return;
}

/**
 *  @brief  从UART读取数据，根据配置的模式分发到具体实现
 *  @param[in]  handle    UART句柄
 *  @param[in]  buf       接收缓冲区
 *  @param[in]  size      要读取的字节数
 *  @param[out] bytesRead 实际读取字节数
 *  @return 状态码
 */
int_fast16_t UART_read(UART_Handle handle, void *buf, size_t size, size_t *bytesRead)
{
    UART_Object *object = UART_Obj_Ptr(handle);
    int_fast16_t result;

    result = object->supportFxns->read(handle, buf, size, bytesRead);
    if (bytesRead)
    {
        /* 出错时将bytesRead置0 */
        *bytesRead = result < 0 ? 0 : *bytesRead;
    }

    return result < 0 ? result : UART_STATUS_SUCCESS;
}

/**
 *  @brief  向UART写入数据，根据配置的模式分发到具体实现
 *  @param[in]  handle        UART句柄
 *  @param[in]  buf           待发送数据缓冲区
 *  @param[in]  size          要发送的字节数
 *  @param[out] bytesWritten  实际发送字节数
 *  @return 状态码
 */
int_fast16_t UART_write(UART_Handle handle, const void *buf, size_t size, size_t *bytesWritten)
{
    UART_Object *object = UART_Obj_Ptr(handle);
    int_fast16_t result;

    result = object->supportFxns->write(handle, buf, size, bytesWritten);

    if (bytesWritten)
    {
        /* 出错时将bytesWritten置0 */
        *bytesWritten = result < 0 ? 0 : *bytesWritten;
    }

    return result < 0 ? result : UART_STATUS_SUCCESS;
}

/**
 *  @brief  带超时的UART读操作
 *  @param[in]  handle    UART句柄
 *  @param[in]  buf       接收缓冲区
 *  @param[in]  size      要读取的字节数
 *  @param[out] bytesRead 实际读取字节数
 *  @param[in]  timeout   超时系统时钟滴答数
 *  @return 状态码
 */
int_fast16_t UART_readTimeout(UART_Handle handle, void *buf, size_t size, size_t *bytesRead, uint32_t timeout)
{
    UART_Object *object                = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(object);
    uint8_t *data                      = (uint8_t *) (buf);
    int_fast16_t count                 = 0;
    int_fast16_t result;
    int_fast16_t bytesToRead;

    /* 回调模式不支持超时读取 */
    if (object->readMode == UART_Mode_CALLBACK)
    {
        return UART_STATUS_EINVALID;
    }

    /* 根据返回模式决定最小读取字节数 */
    bytesToRead = object->returnMode == UART_ReadReturnMode_FULL ? size : 1;

    while (1)
    {
        /* 以非阻塞方式尝试读取 */
        result = UART_readBufferedMode(handle, data, size, bytesRead, UART_Mode_NONBLOCKING);
        if (result >= 0)
        {
            count += *bytesRead;
            size -= *bytesRead;
            data += *bytesRead;
        }
        if (count >= bytesToRead)
        {
            break;
        }
        /* 等待接收信号量，超时则退出 */
        if (SemaphoreP_TIMEOUT == SemaphoreP_pend(buffersObject->rxSem, timeout))
        {
            object->readTimedOut = true;
            break;
        }
    }
    if (object->readMode != UART_Mode_CALLBACK)
    {
        object->readInUse = false;
    }
    if (bytesRead)
    {
        *bytesRead = count;
    }
    return (count < size) ? UART_STATUS_ETIMEOUT : UART_STATUS_SUCCESS;
}

/**
 *  @brief  带超时的UART写操作
 *  @param[in]  handle        UART句柄
 *  @param[in]  buf           待发送数据缓冲区
 *  @param[in]  size          要发送的字节数
 *  @param[out] bytesWritten  实际发送字节数
 *  @param[in]  timeout       超时系统时钟滴答数
 *  @return 状态码
 */
int_fast16_t UART_writeTimeout(UART_Handle handle, const void *buf,
    size_t size, size_t *bytesWritten, uint32_t timeout)
{
    UART_Object *object                = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(object);
    const uint8_t *data                = (const uint8_t *) (buf);
    int_fast16_t count                 = 0;
    int_fast16_t result;

    /* 回调模式不支持超时写入 */
    if (object->readMode == UART_Mode_CALLBACK)
    {
        return UART_STATUS_EINVALID;
    }

    while (1)
    {
        /* 以非阻塞方式尝试写入 */
        result = UART_writeBufferedMode(handle, data, size, bytesWritten, UART_Mode_NONBLOCKING);
        if (result > 0)
        {
            count += result;
            size -= result;
            data += result;
        }
        if (count >= size)
        {
            break;
        }
        /* 等待发送信号量，超时则退出 */
        if (SemaphoreP_TIMEOUT == SemaphoreP_pend(buffersObject->txSem, timeout))
        {
            object->writeTimedOut = true;
            break;
        }
    }
    /* 回调模式下不在此处清除writeInUse标志 */
    if (object->writeMode != UART_Mode_CALLBACK)
    {
        object->writeInUse = false;
    }
    if (bytesWritten)
    {
        *bytesWritten = count;
    }
    return (count < size) ? UART_STATUS_ETIMEOUT : UART_STATUS_SUCCESS;
}

/**
 *  @brief  取消正在进行的UART读操作
 *  @param[in]  handle  UART句柄
 */
void UART_readCancel(UART_Handle handle)
{
    UART_Object *object = UART_Obj_Ptr(handle);
    uintptr_t key;

    key = HwiP_disable();

    if (object->readMode == UART_Mode_CALLBACK)
    {
        /* 回调模式：调用回调并清除使用标志 */
        UART_Callback_Object *callbackObject = UART_callbackObject(object);

        if (object->readInUse)
        {
            uint8_t *readBuf   = callbackObject->readBuf;
            uint16_t readSize  = callbackObject->readSize;
            uint16_t bytesRead = callbackObject->bytesRead;

            /* 清除使用标志以便回调可设置新的读操作 */
            object->readInUse = false;

            callbackObject->readCallback(handle, readBuf, bytesRead,
                object->userArg, readSize != bytesRead ? UART_STATUS_ECANCELLED : UART_STATUS_SUCCESS);
        }
        HwiP_restore(key);
    }
    else
    {
        /* 阻塞/非阻塞模式：设置取消标志并释放信号量 */
        UART_Buffers_Object *buffersObject = UART_buffersObject(object);
        object->readCancel                 = true;

        HwiP_restore(key);
        SemaphoreP_post(buffersObject->rxSem);
    }
}

/**
 *  @brief  取消正在进行的UART写操作
 *  @param[in]  handle  UART句柄
 */
void UART_writeCancel(UART_Handle handle)
{
    UARTMSP_HWAttrs *hwAttrs           = UART_HWAttrs_Ptr(handle);
    UART_Object *object = UART_Obj_Ptr(handle);
    UART_Buffers_Object *buffersObject = UART_buffersObject(object);
    UART_Callback_Object *callbackObject = UART_callbackObject(object);
    uintptr_t key;

    key = HwiP_disable();

    if (!object->writeCancel)
    {
        object->writeCancel = true;

#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G1X0X_G3X0X)||(DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L122X_L222X)|| \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX51X) || (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L111X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G352X))
        /* 停止DMA传输 */
        UARTMSP_dmaStopTx(handle);
#endif
        if (object->writeInUse)
        {
            /* 等待TX FIFO清空 */
            while (!DL_UART_isTXFIFOEmpty(hwAttrs->regs)) {}

            /* 阻塞模式：唤醒被阻塞的写任务 */
            if (object->writeMode == UART_Mode_BLOCKING)
            {
                SemaphoreP_post(&buffersObject->txSem);
            }

            /* 回调模式：调用写回调函数 */
            if (object->writeMode == UART_Mode_CALLBACK)
            {
                object->writeInUse = false;
                callbackObject->writeCount = 0;
                callbackObject->writeCallback(handle,
                                      (void *)callbackObject->writeBuf,
                                      callbackObject->bytesWritten,
                                      callbackObject->userArg,
                                      UART_STATUS_ECANCELLED);
            }
        }
    }

    HwiP_restore(key);
}

/* clang-format on */
