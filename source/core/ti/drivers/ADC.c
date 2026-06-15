/*
 * Copyright (c) 2016-2023, Texas Instruments Incorporated
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
 *  ======== ADC.c ========
 *  ADC 模数转换驱动公共层实现
 *  通过函数表分派到具体硬件驱动（如 ADCMSPM0），提供统一的
 *  初始化、打开、关闭、采样及微伏转换接口。
 */

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

#include <ti/drivers/ADC.h>
#include <ti/drivers/dpl/HwiP.h>

extern const ADC_Config ADC_config[];
extern const uint_least8_t ADC_count;

/* Default ADC parameters structure 默认 ADC 参数 */
const ADC_Params ADC_defaultParams =
{
    NULL,   /* custom: 无自定义参数 */
    true,   /* isProtected: 默认使用信号量保护 */
};

static bool isInitialized = false; /* 驱动初始化标志 */

/**
 * @brief 关闭 ADC 驱动实例，释放资源。
 * @param handle ADC_open() 返回的句柄。
 */
void ADC_close(ADC_Handle handle)
{
    /* 通过函数表调用具体驱动的关闭实现 */
    ((ADC_FxnTable *)handle->fxnTablePtr)->closeFxn(handle);
}

/**
 * @brief 对 ADC 驱动实例执行特定控制命令。
 * @param handle ADC_open() 返回的句柄。
 * @param cmd    设备特定命令码。
 * @param arg    与命令配合使用的可选参数。
 * @return 成功返回 ADC_STATUS_SUCCESS，负值表示失败。
 */
int_fast16_t ADC_control(ADC_Handle handle, uint_fast16_t cmd, void *arg)
{
    /* 通过函数表调用具体驱动的控制实现 */
    return (((ADC_FxnTable *)handle->fxnTablePtr)->controlFxn(handle, cmd, arg));
}

/**
 * @brief 执行单通道 ADC 采样转换。
 * @param handle ADC_open() 返回的句柄。
 * @param value  存放转换结果的指针。
 * @return 成功返回 ADC_STATUS_SUCCESS，失败返回 ADC_STATUS_ERROR。
 */
int_fast16_t ADC_convert(ADC_Handle handle, uint16_t *value)
{
    /* 通过函数表调用具体驱动的单通道转换实现 */
    return (handle->fxnTablePtr->convertFxn(handle, value));
}

/**
 * @brief 执行多通道 ADC 采样转换。
 * @param handleList   ADC 句柄列表。
 * @param dataBuffer   存放各通道转换结果的缓冲区。
 * @param channelCount 通道数量。
 * @return 成功返回 ADC_STATUS_SUCCESS，失败返回 ADC_STATUS_ERROR。
 */
int_fast16_t ADC_convertChain(ADC_Handle *handleList, uint16_t *dataBuffer, uint8_t channelCount)
{
    /* 取首个句柄的函数表执行多通道转换 */
    return (handleList[0]->fxnTablePtr->convertChainFxn(handleList, dataBuffer, channelCount));
}

/**
 * @brief 将 ADC 原始采样值转换为微伏值。
 * @param handle   ADC_open() 返回的句柄。
 * @param adcValue ADC_convert() 返回的原始值。
 * @return 转换后的微伏值。
 */
uint32_t ADC_convertToMicroVolts(ADC_Handle handle, uint16_t adcValue)
{
    /* 通过函数表调用具体驱动的微伏转换实现 */
    return (handle->fxnTablePtr->convertToMicroVolts(handle, adcValue));
}

/**
 * @brief 初始化 ADC 驱动，必须在其他 ADC API 之前调用。
 */
void ADC_init(void)
{
    uint_least8_t i;
    uint_fast32_t key;

    /* 关中断，防止重复初始化 */
    key = HwiP_disable();

    if (!isInitialized)
    {
        isInitialized = (bool)true;

        /* 遍历所有 ADC 配置，逐一调用各驱动的初始化函数 */
        for (i = 0; i < ADC_count; i++)
        {
            ADC_config[i].fxnTablePtr->initFxn((ADC_Handle) & (ADC_config[i]));
        }
    }

    /* 恢复中断状态 */
    HwiP_restore(key);
}

/**
 * @brief 打开并初始化指定索引的 ADC 外设。
 * @param index ADC_Config[] 数组中的索引。
 * @param params 已初始化的 ADC_Params 指针，NULL 则使用默认值。
 * @return 成功返回句柄，失败返回 NULL。
 */
ADC_Handle ADC_open(uint_least8_t index, ADC_Params *params)
{
    ADC_Handle handle = NULL;

    /* 检查驱动已初始化且索引有效 */
    if (isInitialized && (index < ADC_count))
    {
        /* params 为 NULL 时使用默认参数 */
        if (params == NULL)
        {
            params = (ADC_Params *)&ADC_defaultParams;
        }

        /* 获取配置句柄并调用具体驱动的打开实现 */
        handle = (ADC_Handle) & (ADC_config[index]);
        handle = handle->fxnTablePtr->openFxn(handle, params);
    }

    return (handle);
}

/**
 * @brief 将 ADC_Params 结构体初始化为默认值。
 * @param params ADC_Params 结构体指针。
 */
void ADC_Params_init(ADC_Params *params)
{
    /* 用默认参数结构体赋值 */
    *params = ADC_defaultParams;
}
/* clang-format on */
