/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * *  Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer. *
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
 *  ======== I2CTarget.c ========
 *  I2CTarget 通用驱动实现，提供模块初始化和参数默认值设置
 */
/* clang-format off */

#include <stdint.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/I2CTarget.h>

extern const I2CTarget_Config I2CTarget_config[];
extern const uint_least8_t I2CTarget_count;

/* Default I2CTarget parameters structure 默认 I2CTarget 参数结构体 */
const I2CTarget_Params I2CTarget_defaultParams = {
    NULL, /* eventCallbackFxn 事件回调函数，默认为空 */
    I2C_TARGET_OWN_ADDRESS, /* targetAddress 从机地址，默认 0x48 */
    NULL  /* custom 自定义参数，默认为空 */
};

/*
 *  ======== I2CTarget_init ========
 *  初始化 I2CTarget 模块，当前为空实现
 */
void I2CTarget_init(void)
{
    /* Do nothing */
}

/*
 *  ======== I2CTarget_Params_init =======
 *  将 I2CTarget_Params 结构体初始化为默认参数值
 */
void I2CTarget_Params_init(I2CTarget_Params *params)
{
    *params = I2CTarget_defaultParams;
}
/* clang-format on */
