/*
 * Copyright (c) 2015-2023, Texas Instruments Incorporated
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
 *  ======== I2C.c ========
 *  I2C 驱动通用层实现。提供 I2C_init、I2C_transfer、I2C_cancel 等
 *  平台无关的接口，通过 I2CSupport 调用具体的硬件操作。
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <ti/drivers/I2C.h>
#include <ti/drivers/i2c/I2CSupport.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

/* 默认 I2C 参数结构体 */
const I2C_Params I2C_defaultParams = {
    I2C_MODE_BLOCKING, /* transferMode */
    NULL,              /* transferCallbackFxn */
    I2C_100kHz,        /* bitRate */
    NULL               /* custom */
};

/*
 *  ======== I2C_cancel ========
 *  取消当前及排队的 I2C 传输。在总线上产生 STOP 条件。
 */
void I2C_cancel(I2C_Handle handle)
{
    I2C_Object *object         = handle->object;
    I2C_HWAttrs const *hwAttrs = handle->hwAttrs;
    uintptr_t key;

    key = HwiP_disable(); /* 关中断，进入临界区 */

    /* Return if no transfer in progress / 无传输进行中则直接返回 */
    if (!object->headPtr) {
        HwiP_restore(key);
        return;
    }

    I2CSupport_controllerFinish(hwAttrs); /* 产生 STOP 条件 */

    /* Set current transaction as canceled / 标记当前事务为已取消 */
    object->currentTransaction->status = I2C_STATUS_CANCEL;

    HwiP_restore(key); /* 恢复中断 */
}

/*
 *  ======== I2C_init ========
 *  初始化 I2C 驱动。当前为空实现，保留用于未来扩展。
 */
void I2C_init(void)
{
    /* Do nothing / 当前无操作 */
}

/*
 *  ======== I2C_Params_init =======
 *  将 I2C_Params 结构体初始化为默认值。
 */
void I2C_Params_init(I2C_Params *params)
{
    *params = I2C_defaultParams; /* 使用默认参数填充 */
}

/*
 *  ======== I2C_transfer ========
 *  执行 I2C 传输（永久等待）。封装 I2C_transferTimeout。
 */
bool I2C_transfer(I2C_Handle handle, I2C_Transaction *transaction)
{
    int_fast16_t result =
        (I2C_transferTimeout(handle, transaction, I2C_WAIT_FOREVER));

    /* 根据返回状态转换为布尔值 */
    if (result == I2C_STATUS_SUCCESS) {
        return (true);
    } else {
        return (false);
    }
}

/*
 *  ======== I2C_transferTimeout ========
 *  带超时的 I2C 传输。处理阻塞/回调模式、事务排队及超时逻辑。
 */
int_fast16_t I2C_transferTimeout(
    I2C_Handle handle, I2C_Transaction *transaction, uint32_t timeout)
{
    I2C_Object *object         = handle->object;
    I2C_HWAttrs const *hwAttrs = handle->hwAttrs;
    uintptr_t key;
    int_fast16_t ret;

    /* Check if anything needs to be written or read / 检查读写计数是否均为 0 */
    if ((!transaction->writeCount) && (!transaction->readCount)) {
        transaction->status = I2C_STATUS_INVALID_TRANS;
        /* Nothing to write or read */
        return (transaction->status);
    }

    key = HwiP_disable(); /* 关中断 */

    /* If callback mode, determine if transfer in progress / 回调模式下检查是否有传输进行中 */
    if (object->transferMode == I2C_MODE_CALLBACK && object->headPtr) {
        /*
         * Queued transactions are being canceled. Can't allow additional
         * transactions to be queued.
         * 队列事务正在被取消，不允许追加新事务。
         */
        if (object->headPtr->status == I2C_STATUS_CANCEL) {
            transaction->status = I2C_STATUS_INVALID_TRANS;
            ret                 = transaction->status;
        }
        /* Transfer in progress */
        else {
            /*
             * Update the message pointed by the tailPtr to point to the next
             * message in the queue
             * 将新事务追加到队列尾部。
             */
            object->tailPtr->nextPtr = transaction;

            /* Update the tailPtr to point to the last message */
            object->tailPtr = transaction;

            /* Set queued status / 设置排队状态 */
            transaction->status = I2C_STATUS_QUEUED;

            ret = I2C_STATUS_SUCCESS;
        }

        HwiP_restore(key);
        return (ret);
    }

    /* Store the headPtr indicating I2C is in use / 设置头尾指针标记 I2C 正在使用 */
    object->headPtr = transaction;
    object->tailPtr = transaction;

    /* In blocking mode, transactions can queue on the I2C mutex */
    transaction->status = I2C_STATUS_QUEUED;

    HwiP_restore(key); /* 恢复中断 */

    /* Get the lock for this I2C handle / 获取 I2C 互斥锁 */
    if (SemaphoreP_pend(&(object->mutex), SemaphoreP_NO_WAIT) ==
        SemaphoreP_TIMEOUT) {
        /* We were unable to get the mutex in CALLBACK mode */
        if (object->transferMode == I2C_MODE_CALLBACK) {
            /*
             * Recursively call transfer() and attempt to place transaction
             * on the queue. This may only occur if a thread is preempted
             * after restoring interrupts and attempting to grab this mutex.
             * 回调模式下获取互斥锁失败时递归重试。
             */
            return (I2C_transferTimeout(handle, transaction, timeout));
        }

        /* Wait for the I2C lock. If it times-out before we retrieve it, then
         * return false to cancel the transaction.
         * 阻塞等待互斥锁，超时则返回超时状态。 */
        if (SemaphoreP_pend(&(object->mutex), timeout) == SemaphoreP_TIMEOUT) {
            transaction->status = I2C_STATUS_TIMEOUT;
            return (I2C_STATUS_TIMEOUT);
        }
    }

    if (object->transferMode == I2C_MODE_BLOCKING) {
        /*
         * In the case of a timeout, It is possible for the timed-out transaction
         * to call the hwi function and post the object->transferComplete Semaphore
         * To clear this, we simply do a NO_WAIT pend on (binary)
         * object->transferComplete, so that it resets the Semaphore count.
         * 超时情况下清除可能残留的信号量。
         */
        SemaphoreP_pend(&(object->transferComplete), SemaphoreP_NO_WAIT);
    }

    I2CSupport_powerSetConstraint(); /* 设置电源约束，阻止进入待机 */

    /*
     * I2CSupport_primeTransfer is a longer process and
     * protection is needed from the I2C interrupt
     * 准备传输，期间禁用 I2C 中断以保护临界区。
     */
    HwiP_disableInterrupt(hwAttrs->intNum);
    ret = I2CSupport_primeTransfer(handle, transaction);
    HwiP_enableInterrupt(hwAttrs->intNum);

    if (ret != I2C_STATUS_SUCCESS) {
        /* Release disallow constraint. / 传输启动失败，释放电源约束 */
        I2CSupport_powerRelConstraint();
    } else if (object->transferMode == I2C_MODE_BLOCKING) {
        /* Wait for the primed transfer to complete / 阻塞等待传输完成 */
        if (SemaphoreP_pend(&(object->transferComplete), timeout) ==
            SemaphoreP_TIMEOUT) {
            key = HwiP_disable();

            /*
             * Protect against a race condition in which the transfer may
             * finish immediately after the timeout. If this occurs, we
             * will preemptively consume the semaphore on the next initiated
             * transfer.
             * 防止超时与传输完成之间的竞态条件。
             */
            if (object->headPtr) {
                /*
                 * It's okay to call cancel here since this is blocking mode.
                 * Cancel will generate a STOP condition and immediately
                 * return.
                 * 阻塞模式下取消传输，产生 STOP 条件。
                 */
                I2C_cancel(handle);

                HwiP_restore(key);

                /*
                 *  We must wait until the STOP interrupt occurs. When this
                 *  occurs, the semaphore will be posted. Since the STOP
                 *  condition will finish the current burst, we can't
                 *  unblock the object->mutex until this occurs.
                 *
                 *  This may block forever if the target holds the clock line--
                 *  rendering the I2C bus un-usable.
                 *  等待 STOP 中断完成。若目标设备拉住时钟线可能永久阻塞。
                 */
                SemaphoreP_pend(
                    &(object->transferComplete), SemaphoreP_WAIT_FOREVER);

                transaction->status = I2C_STATUS_TIMEOUT;
            } else {
                HwiP_restore(key);
            }
        }

        ret = transaction->status;
    }

    /* Release the lock for this particular I2C handle / 释放互斥锁 */
    SemaphoreP_post(&(object->mutex));

    /* Return status / 返回传输状态 */
    return (ret);
}
