/*
 * Copyright (c) 2023, Texas Instruments Incorporated
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
 *  ======== GPIO.c ========
 *  GPIO 驱动实现文件，提供回调绑定/查询、用户参数存取及引脚配置重置等功能。
 */
#include <stdint.h>
#include <string.h>

#include <ti/drivers/GPIO.h>

extern GPIO_Config GPIO_config;

/**
 *  @brief      为指定 GPIO 引脚绑定回调函数。
 *
 *  @param      index       GPIO 引脚索引
 *  @param      callback    回调函数地址，传入 NULL 可清除已绑定的回调
 */
void GPIO_setCallback(uint_least8_t index, GPIO_CallbackFxn callback)
{
    /*
     * Only update callbacks entry if different.
     * This allows the callbacks array to be in flash for static systems.
     * 仅在回调发生变化时更新，以支持回调数组位于 Flash 的静态系统。
     */
    if (index != GPIO_INVALID_INDEX &&
        GPIO_config.callbacks[index] != callback) {
        GPIO_config.callbacks[index] = callback;
    }
}

/**
 *  @brief      获取指定 GPIO 引脚当前绑定的回调函数。
 *
 *  @param      index       GPIO 引脚索引
 *
 *  @return     回调函数指针，若未绑定则返回 NULL
 */
GPIO_CallbackFxn GPIO_getCallback(uint_least8_t index)
{
    return GPIO_config.callbacks[index];
}

/**
 *  @brief      设置指定 GPIO 引脚的用户参数。
 *
 *  @param      index       GPIO 引脚索引
 *  @param      arg         指向用户自定义对象的指针
 */
void GPIO_setUserArg(uint_least8_t index, void *arg)
{
    /* 拒绝无效索引，防止越界写入 */
    if (index != GPIO_INVALID_INDEX) {
        GPIO_config.userArgs[index] = arg;
    }
}

/**
 *  @brief      获取指定 GPIO 引脚的用户参数。
 *
 *  @param      index       GPIO 引脚索引
 *
 *  @return     由 GPIO_setUserArg() 设置的用户对象指针
 */
void *GPIO_getUserArg(uint_least8_t index)
{
    return GPIO_config.userArgs[index];
}

/**
 *  @brief      将指定 GPIO 引脚的配置恢复为默认值。
 *
 *  @param      index       GPIO 引脚索引
 */
void GPIO_resetConfig(uint_least8_t index)
{
    /* 拒绝无效索引 */
    if (index != GPIO_INVALID_INDEX) {
        GPIO_disableInt(index);                          /* 先禁用中断 */
        GPIO_setConfig(index, GPIO_config.configs[index]); /* 恢复默认引脚配置 */
        GPIO_setCallback(index, NULL);                   /* 清除回调 */
        GPIO_setUserArg(index, NULL);                    /* 清除用户参数 */
    }
}
