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
 *  ======== SPI.c ========
 *  SPI 驱动通用抽象层实现，通过函数表分发调用到具体驱动实现。
 */
/* clang-format off */
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/SPI.h>

extern const SPI_Config SPI_config[];
extern const uint_least8_t SPI_count;

/* Default SPI parameters structure */
/* SPI 默认参数结构体 */
const SPI_Params SPI_defaultParams = {
    /*! transferMode */
    SPI_MODE_BLOCKING,
    /*! transferTimeout */
    SPI_WAIT_FOREVER,
    /*! transferCallbackFxn */
    NULL,
    /*! mode */
    SPI_CONTROLLER,
    /*! bitRate */
    500000,
    /*! dataSize */
    8,
    /*! frameFormat */
    SPI_MOTO4_POL0_PHA0,
    /*! parity */
    SPI_PARITY_NONE,
    /*! bit order */
    SPI_BIT_ORDER_MSB_FIRST,
    /*! custom */
    NULL
};

static bool isInitialized = false;

/*
 *  ======== SPI_close ========
 */
/** @brief 关闭 SPI 外设，释放资源。通过函数表调用具体实现。 */
void SPI_close(SPI_Handle handle)
{
    handle->fxnTablePtr->closeFxn(handle);
}

/*
 *  ======== SPI_control ========
 */
/** @brief 执行 SPI 控制命令。通过函数表调用具体实现。 */
int_fast16_t SPI_control(SPI_Handle handle, uint_fast16_t cmd, void *controlArg)
{
    return (handle->fxnTablePtr->controlFxn(handle, cmd, controlArg));
}

/*
 *  ======== SPI_init ========
 */
/**
 * @brief 初始化 SPI 模块。禁用中断防止重入，遍历 SPI_config 数组
 *        调用每个驱动实例的初始化函数。仅执行一次。
 */
void SPI_init(void)
{
    uint_least8_t i;
    uint_fast8_t key;

    key = HwiP_disable();       /* 关闭中断，防止重入 */

    if (!isInitialized)
    {
        isInitialized = (bool)true;

        /* Call each driver's init function */
        /* 遍历所有 SPI 配置，逐一调用其初始化函数 */
        for (i = 0; i < SPI_count; i++)
        {
            SPI_config[i].fxnTablePtr->initFxn((SPI_Handle) &SPI_config[i]);
        }
    }

    HwiP_restore(key);          /* 恢复中断状态 */
}

/*
 *  ======== SPI_open ========
 */
/**
 * @brief 打开指定索引的 SPI 外设。参数为 NULL 时使用默认值。
 *
 * @param index  SPI_config 数组索引
 * @param params 参数指针，为 NULL 使用默认参数
 * @return 成功返回句柄，失败返回 NULL
 */
SPI_Handle SPI_open(uint_least8_t index, SPI_Params *params)
{
    SPI_Handle handle = NULL;

    if (isInitialized && (index < SPI_count))
    {
        /* If params are NULL use defaults */
        if (params == NULL)
        {
            params = (SPI_Params *)&SPI_defaultParams;  /* 参数为空则使用默认值 */
        }

        /* Get handle for this driver instance */
        handle = (SPI_Handle) &SPI_config[index];
        handle = handle->fxnTablePtr->openFxn(handle, params);  /* 调用具体实现的 open */
    }

    return (handle);
}

/*
 *  ======== SPI_Params_init ========
 */
/** @brief 将 SPI_Params 结构体初始化为默认参数值。 */
void SPI_Params_init(SPI_Params *params)
{
    *params = SPI_defaultParams;
}

/*
 *  ======== SPI_transfer ========
 */
/** @brief 执行 SPI 数据传输事务。通过函数表调用具体实现。 */
bool SPI_transfer(SPI_Handle handle, SPI_Transaction *transaction)
{
    return (handle->fxnTablePtr->transferFxn(handle, transaction));
}

/*
 *  ======== SPI_transferCancel ========
 */
/** @brief 取消正在进行的 SPI 传输。通过函数表调用具体实现。 */
void SPI_transferCancel(SPI_Handle handle)
{
    handle->fxnTablePtr->transferCancelFxn(handle);
}
/* clang-format on */
