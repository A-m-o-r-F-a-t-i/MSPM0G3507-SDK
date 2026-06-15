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

/**
 *  @brief MSPM0 系列 GPIO 驱动实现文件
 *
 *  本文件实现了 TI MSPM0 系列 MCU 的 GPIO 驱动，包括:
 *  - GPIO 引脚读写、翻转
 *  - 中断使能/禁止/清除
 *  - 引脚复用与功能配置
 *  - 硬件中断(HWI)回调分发
 *
 *  支持的器件族: MSPM0L11XX/L13XX、MSPM0G1X0X/G3X0X、MSPM0L111X、MSPM0L122X/L222X、MSPM0GX51X、MSPM0G352X
 */

#include <stdbool.h>
#include <stdint.h>

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOMSPM0.h>

#include <ti/driverlib/driverlib.h>

/* Internal boolean to confirm that GPIO_init() has been called */
/* 内部标志位，标记 GPIO_init() 是否已调用 */
static bool initCalled = false;

/* Link to config values defined by SysConfig */
/* SysConfig 生成的配置结构体和引脚范围 */
extern GPIO_Config GPIO_config;
extern const uint_least8_t GPIO_pinLowerBound;
extern const uint_least8_t GPIO_pinUpperBound;

/*
 * Bitmask to determine if a Hwi has been created/constructed
 * for a port already
 */
/* 位掩码: 记录哪些端口已创建 HWI 中断，避免重复创建 */
static uint8_t portHwiCreatedBitMask = 0;

/* Used to contain device specific port information */
/* 端口配置结构体: 存储每个 GPIO 端口的中断号和基地址 */
typedef struct {
    const IRQn_Type interruptNum;   /* 端口中断号 */
    const uint32_t baseAddress;     /* 端口寄存器基地址 */
} PortConfig;

#if (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L11XX_L13XX)

/* MSPM0L11XX/L13XX: 仅 1 个 GPIO 端口 (PORTA) */
#define NUM_PORTS (1)
#define PORTA_INDEX (0)

/* Port information */
static const PortConfig portConfigs[NUM_PORTS] = {
    {GPIOA_INT_IRQn, GPIOA_BASE},
};

#elif ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G1X0X_G3X0X) || \
       (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L111X))

/* MSPM0G1X0X/G3X0X、MSPM0L111X: 2 个 GPIO 端口 (PORTA, PORTB) */
#define NUM_PORTS (2)
#define PORTA_INDEX (0)
#define PORTB_INDEX (1)

/* Port information */
static const PortConfig portConfigs[NUM_PORTS] = {
    {GPIOA_INT_IRQn, GPIOA_BASE},
    {GPIOB_INT_IRQn, GPIOB_BASE},
};

#elif ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L122X_L222X) || \
       (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX51X) ||       \
       (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G352X))

/* MSPM0L122X/L222X、MSPM0GX51X、MSPM0G352X: 3 个 GPIO 端口 (PORTA, PORTB, PORTC) */
#define NUM_PORTS (3)
#define PORTA_INDEX (0)
#define PORTB_INDEX (1)
#define PORTC_INDEX (2)

/* Port information */
static const PortConfig portConfigs[NUM_PORTS] = {
    {GPIOA_INT_IRQn, GPIOA_BASE},
    {GPIOB_INT_IRQn, GPIOB_BASE},
    {GPIOC_INT_IRQn, GPIOC_BASE},
};

#else

#error "Drivers is not supported on this device yet"

#endif

/* 每个 GPIO 端口最多支持 32 个引脚 */
#define MAX_PINS_PER_GPIO_PORT (32)

/* Returns the GPIO port number or port config struct */
/* 根据全局引脚索引计算端口 ID 和端口配置 */
#define indexToPortId(pinIndex) (pinIndex / MAX_PINS_PER_GPIO_PORT)
#define indexToPortConfig(pinIndex) (portConfigs[indexToPortId(pinIndex)])

/* Generates a per-port mask for a pin, e.g. the second pin in a port should be 0x10 */
/* 根据全局引脚索引生成端口内位掩码，例如端口内第 2 个引脚对应 0x02 */
#define indexToPortMask(pinIndex) (1U << (pinIndex % MAX_PINS_PER_GPIO_PORT))

/* 全中断掩码，用于批量读取/清除所有中断标志 */
#define ALL_INTERRUPTS_MASK (0xFFFFFFFFU)

/**
 *  @brief  清除指定 GPIO 引脚的中断标志 GPIO_clearInt
 *
 *  @param  index   全局引脚索引号
 *
 *  清除对应引脚的中断挂起标志，通常在中断服务函数末尾调用。
 */
void GPIO_clearInt(uint_least8_t index)
{
    if (index == GPIO_INVALID_INDEX) {
        return;
    }

    /* Clear GPIO interrupt flag */
    /* 根据索引定位端口基地址和引脚掩码，清除中断标志 */
    DL_GPIO_clearInterruptStatus(
        (GPIO_Regs *) (indexToPortConfig(index).baseAddress),
        indexToPortMask(index));
}

/**
 *  @brief  禁用指定 GPIO 引脚的中断 GPIO_disableInt
 *
 *  @param  index   全局引脚索引号
 *
 *  禁用引脚中断、清除 NVIC 挂起位，并更新配置表中的中断掩码。
 *  操作期间通过 HwiP_disable 保证原子性。
 */
void GPIO_disableInt(uint_least8_t index)
{
    if (index == GPIO_INVALID_INDEX) {
        return;
    }

    /* Update the config table and disable interrupts */
    /* 关中断进入临界区 */
    uintptr_t key = HwiP_disable();

    /* 清除配置表中的中断使能位 */
    GPIO_config.configs[index] &= ~GPIO_CFG_INT_MASK;
    DL_GPIO_disableInterrupt(
        (GPIO_Regs *) (indexToPortConfig(index).baseAddress),
        indexToPortMask(index));

    /* Clear pending interrupts at the NVIC level */
    /* 清除 NVIC 中的挂起中断，防止误触发 */
    NVIC_ClearPendingIRQ(indexToPortConfig(index).interruptNum);

    /* 恢复中断 */
    HwiP_restore(key);
}

/**
 *  @brief  使能指定 GPIO 引脚的中断 GPIO_enableInt
 *
 *  @param  index   全局引脚索引号
 *
 *  使能引脚中断并更新配置表中的中断掩码。
 *  操作期间通过 HwiP_disable 保证原子性。
 */
void GPIO_enableInt(uint_least8_t index)
{
    if (index == GPIO_INVALID_INDEX) {
        return;
    }

    /* Update the config table and enable interrupts */
    /* 关中断进入临界区 */
    uintptr_t key = HwiP_disable();

    /* 设置配置表中的中断使能位 */
    GPIO_config.configs[index] |= GPIO_CFG_INT_MASK;
    DL_GPIO_enableInterrupt(
        (GPIO_Regs *) (indexToPortConfig(index).baseAddress),
        indexToPortMask(index));

    /* 恢复中断 */
    HwiP_restore(key);
}

/**
 *  @brief  GPIO 中断硬件中断服务函数 GPIO_hwiIntFxn
 *
 *  @param  portIndex   端口索引(由 HWI 创建时传入)
 *
 *  遍历所有已使能中断的 GPIO 端口，读取并清除中断标志，
 *  然后调用对应引脚注册的回调函数。
 *  由于 GPIOA/GPIOB/GPIOC 可能共享同一中断号，需逐一检查。
 */
void GPIO_hwiIntFxn(uintptr_t portIndex)
{
    uint32_t pinIndex            = 0;
    uint32_t flagIndex           = 0;
    uint32_t pendingIntMaskGPIOA = 0;

    /* Determine if a GPIO interrupt occurred, not another group element */
    /* 检查中断组 1 中是否有 GPIO 中断挂起 */
    uint32_t groupIIDX = DL_Interrupt_getPendingGroup(DL_INTERRUPT_GROUP_1);
    if ((groupIIDX == DL_INTERRUPT_GROUP1_IIDX_GPIOA) ||
        (groupIIDX == DL_INTERRUPT_GROUP1_IIDX_GPIOB) ||
        (groupIIDX == DL_INTERRUPT_GROUP1_IIDX_GPIOC)) {
        /*
         * Find out which pins have their interrupt flags set.
         * GPIOA and GPIOB are grouped interrupts with the same
         * interrupt number, and are therefore considered to be the
         * same entry in the Hwi dispatch table. As a workaround,
         * check all pending interrupts on both ports.
         */
        pendingIntMaskGPIOA =
            DL_GPIO_getEnabledInterruptStatus(GPIOA, ALL_INTERRUPTS_MASK);

        /* Clear the set bits at once */
        /* 批量清除 PORTA 所有中断标志 */
        DL_GPIO_clearInterruptStatus(GPIOA, ALL_INTERRUPTS_MASK);

        /* Match the interrupt to its corresponding callback function */
        /* 逐位遍历 PORTA 的挂起中断，调用对应引脚的回调函数 */
        while (pendingIntMaskGPIOA) {
            /* Note MASK_TO_PIN only detects the highest set bit */
            /* 取出最高位的中断标志 */
            flagIndex = GPIO_MASK_TO_PIN(pendingIntMaskGPIOA);
            pendingIntMaskGPIOA &= ~GPIO_PIN_TO_MASK(flagIndex);

            /* Need to go from port index up to global index */
            /* 端口内引脚号转换为全局引脚索引 */
            pinIndex = flagIndex + (PORTA_INDEX * MAX_PINS_PER_GPIO_PORT);

            if (pinIndex >= GPIO_pinLowerBound &&
                pinIndex <= GPIO_pinUpperBound) {
                if (GPIO_config.callbacks[pinIndex] != NULL) {
                    GPIO_config.callbacks[pinIndex](pinIndex);
                }
            }
        }
#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G1X0X_G3X0X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L122X_L222X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX51X) ||       \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L111X) ||       \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G352X))
        uint32_t pendingIntMaskGPIOB = 0;
        pendingIntMaskGPIOB =
            DL_GPIO_getEnabledInterruptStatus(GPIOB, ALL_INTERRUPTS_MASK);
        DL_GPIO_clearInterruptStatus(GPIOB, ALL_INTERRUPTS_MASK);

        while (pendingIntMaskGPIOB) {
            flagIndex = GPIO_MASK_TO_PIN(pendingIntMaskGPIOB);
            pendingIntMaskGPIOB &= ~GPIO_PIN_TO_MASK(flagIndex);

            pinIndex = flagIndex + (PORTB_INDEX * MAX_PINS_PER_GPIO_PORT);

            if (pinIndex >= GPIO_pinLowerBound &&
                pinIndex <= GPIO_pinUpperBound) {
                if (GPIO_config.callbacks[pinIndex] != NULL) {
                    GPIO_config.callbacks[pinIndex](pinIndex);
                }
            }
        }
#endif
#if ((DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0L122X_L222X) || \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0GX51X) ||       \
     (DeviceFamily_PARENT == DeviceFamily_PARENT_MSPM0G352X))
        uint32_t pendingIntMaskGPIOC = 0;
        pendingIntMaskGPIOC =
            DL_GPIO_getEnabledInterruptStatus(GPIOC, ALL_INTERRUPTS_MASK);
        DL_GPIO_clearInterruptStatus(GPIOC, ALL_INTERRUPTS_MASK);

        while (pendingIntMaskGPIOC) {
            flagIndex = GPIO_MASK_TO_PIN(pendingIntMaskGPIOC);
            pendingIntMaskGPIOC &= ~GPIO_PIN_TO_MASK(flagIndex);

            pinIndex = flagIndex + (PORTC_INDEX * MAX_PINS_PER_GPIO_PORT);

            if (pinIndex >= GPIO_pinLowerBound &&
                pinIndex <= GPIO_pinUpperBound) {
                if (GPIO_config.callbacks[pinIndex] != NULL) {
                    GPIO_config.callbacks[pinIndex](pinIndex);
                }
            }
        }
#endif
    }

    return;
}

/**
 *  @brief  初始化 GPIO 驱动 GPIO_init
 *
 *  使用 SysConfig 生成的配置遍历所有引脚，调用 GPIO_setConfig 完成硬件配置，
 *  并注册已定义的回调函数。使用二进制信号量保证多线程安全且仅初始化一次。
 */
void GPIO_init()
{
    uint32_t i;
    uintptr_t key;
    SemaphoreP_Handle sem;
    static SemaphoreP_Handle initSem;

    /* 创建二进制信号量用于保护初始化过程 */
    sem = SemaphoreP_createBinary(1);

    if (sem == NULL) {
        /* Error with creating binary semaphore */
        return;
    }

    /* Disable interrupts for critical region */
    /* 关中断，检查是否首次调用 */
    key = HwiP_disable();

    if (initSem == NULL) {
        /* 首次调用: 保存信号量句柄 */
        initSem = sem;
        HwiP_restore(key);
    } else {
        /* init has already been called */
        /* 非首次调用: 释放多余的信号量 */
        HwiP_restore(key);

        if (sem) {
            SemaphoreP_delete(sem);
        }
    }

    /* Use semaphore to protect init code */
    /* 获取信号量，确保初始化代码串行执行 */
    SemaphoreP_pend(initSem, SemaphoreP_WAIT_FOREVER);

    /* If init has already been called, restore interrupts and return */
    /* 若已初始化则直接返回 */
    if (initCalled) {
        SemaphoreP_post(initSem);
        return;
    }

    /* 遍历所有引脚，应用 SysConfig 生成的配置和回调 */
    for (i = GPIO_pinLowerBound; i < GPIO_pinUpperBound; i++) {
        GPIO_setConfig(i, GPIO_config.configs[i]);

        if (GPIO_config.callbacks[i] != NULL) {
            GPIO_setCallback(i, GPIO_config.callbacks[i]);
        }
    }

    /* 标记初始化完成，释放信号量 */
    initCalled = true;
    SemaphoreP_post(initSem);
}

/**
 *  @brief  配置 GPIO 引脚 GPIO_setConfig
 *
 *  @param  index       全局引脚索引号
 *  @param  pinConfig   引脚配置值(GPIO_CFG_* 宏组合)
 *
 *  @return GPIO_STATUS_SUCCESS 成功; GPIO_STATUS_ERROR 失败
 *
 *  将引脚配置为 GPIO 功能(非外设复用)，内部调用 GPIO_setConfigAndMux。
 */
int_fast16_t GPIO_setConfig(uint_least8_t index, GPIO_PinConfig pinConfig)
{
    return (GPIO_setConfigAndMux(index, pinConfig, GPIO_MUX_GPIO));
}

/**
 *  @brief  配置 GPIO 引脚中断触发方式 GPIO_setInterruptConfig
 *
 *  @param  index       全局引脚索引号
 *  @param  pinConfig   引脚配置值，其中 GPIO_CFG_INT_MASK 字段决定触发方式
 *
 *  根据 pinConfig 中的中断配置设置引脚极性(上升沿/下降沿/双沿)，
 *  并使能或禁止中断。引脚 0-15 使用低极性寄存器，引脚 16-31 使用高极性寄存器。
 */
void GPIO_setInterruptConfig(uint_least8_t index, GPIO_PinConfig pinConfig)
{
    if (index == GPIO_INVALID_INDEX) {
        return;
    }

    GPIO_Regs *port;
    uint32_t pinMask;
    uint32_t pinIndex;
    uint32_t intValue;

    port    = (GPIO_Regs *) (indexToPortConfig(index).baseAddress);
    pinMask = indexToPortMask(index);

    /* 从配置值中提取中断触发类型字段 */
    intValue = (pinConfig & GPIO_CFG_INT_MASK) >> GPIO_CFG_INT_LSB;

    pinIndex = index % MAX_PINS_PER_GPIO_PORT;

    /* Configure the polarity, which is needed to trigger interrupts */
    if (intValue != GPIO_CFG_IN_INT_NONE) {
        /* 设置引脚极性: 引脚 0-15 用低极性寄存器，16-31 用高极性寄存器 */
        if (pinMask < DL_GPIO_PIN_16) {
            DL_GPIO_setLowerPinsPolarity(port, intValue << (pinIndex * 2));
        } else {
            DL_GPIO_setUpperPinsPolarity(
                port, intValue << ((pinIndex - 16) * 2));
        }

        /* Clear interrupt status before enabling */
        /* 使能前先清除残留中断标志 */
        GPIO_clearInt(index);

        GPIO_enableInt(index);
    } else {
        /* 无中断配置则禁止中断 */
        GPIO_disableInt(index);
    }
}

/**
 *  @brief  获取 GPIO 引脚当前配置 GPIO_getConfig
 *
 *  @param  index       全局引脚索引号
 *  @param  pinConfig   输出参数，接收当前引脚配置值
 */
void GPIO_getConfig(uint_least8_t index, GPIO_PinConfig *pinConfig)
{
    *pinConfig = GPIO_config.configs[index];
}

/**
 *  @brief  读取 GPIO 引脚电平 GPIO_read
 *
 *  @param  index   全局引脚索引号
 *
 *  @return 1 表示高电平，0 表示低电平
 */
uint_fast8_t GPIO_read(uint_least8_t index)
{
    /* 读取端口引脚寄存器，判断目标引脚电平 */
    uint32_t value =
        DL_GPIO_readPins((GPIO_Regs *) (indexToPortConfig(index).baseAddress),
            indexToPortMask(index));

    /* 非零返回 1(高电平)，零返回 0(低电平) */
    return value ? 1 : 0;
}

/**
 *  @brief  翻转 GPIO 引脚电平 GPIO_toggle
 *
 *  @param  index   全局引脚索引号
 *
 *  原子操作: 翻转引脚电平并同步更新配置表中的输出状态。
 */
void GPIO_toggle(uint_least8_t index)
{
    if (index == GPIO_INVALID_INDEX) {
        return;
    }

    uintptr_t key;

    GPIO_Regs *port;
    uint32_t pinMask;

    port    = (GPIO_Regs *) (indexToPortConfig(index).baseAddress);
    pinMask = indexToPortMask(index);

    /* Make atomic update */
    /* 关中断保证翻转和配置表更新的原子性 */
    key = HwiP_disable();

    /* 翻转硬件引脚电平 */
    DL_GPIO_togglePins(port, pinMask);

    /* Update config table entry with value written */
    /* 同步翻转配置表中的输出状态位 */
    GPIO_config.configs[index] ^= GPIO_CFG_OUT_HIGH;

    HwiP_restore(key);
}

/**
 *  @brief  写入 GPIO 引脚电平 GPIO_write
 *
 *  @param  index   全局引脚索引号
 *  @param  value   输出值: 非零为高电平，零为低电平
 *
 *  原子操作: 设置或清除引脚电平，并同步更新配置表中的输出状态。
 */
void GPIO_write(uint_least8_t index, unsigned int value)
{
    if (index == GPIO_INVALID_INDEX) {
        return;
    }

    uintptr_t key;

    GPIO_Regs *port;
    uint32_t pinMask;

    port    = (GPIO_Regs *) (indexToPortConfig(index).baseAddress);
    pinMask = indexToPortMask(index);

    /* Disable interrupts to set pins */
    /* 关中断保证写入和配置表更新的原子性 */
    key = HwiP_disable();

    /* Update the pin config table if the value is different */
    /* 仅在状态变化时更新配置表，避免冗余写入 */
    if (value && !(GPIO_config.configs[index] & GPIO_CFG_OUT_HIGH)) {
        GPIO_config.configs[index] |= GPIO_CFG_OUT_HIGH;
    } else if (!value && (GPIO_config.configs[index] & GPIO_CFG_OUT_HIGH)) {
        GPIO_config.configs[index] &= ~GPIO_CFG_OUT_HIGH;
    }

    /* Write the value to the GPIO pins */
    /* 写入硬件引脚: 高电平置位，低电平清零 */
    if (value) {
        DL_GPIO_setPins(port, pinMask);
    } else {
        DL_GPIO_clearPins(port, pinMask);
    }

    HwiP_restore(key);
}

/**
 *  @brief  获取 GPIO 引脚当前复用功能编码 GPIO_getMux
 *
 *  @param  index   全局引脚索引号
 *
 *  @return IOMUX PINCM 寄存器中的引脚功能(PF)字段值
 *
 *  从配置表中取出 PINCM 编号，读取 IOMUX 寄存器返回当前复用功能。
 */
uint32_t GPIO_getMux(uint_least8_t index)
{
    GPIO_PinConfig pinConfig = GPIO_config.configs[index];

    /* Last 16-bits of pinConfig encode the iomux pincm */
    /* 低 8 位编码 IOMUX PINCM 通道号 */
    uint32_t pincm = pinConfig & 0xFF;

    /* Return pin function encoding */
    /* 读取 PINCM 寄存器的引脚功能(PF)字段 */
    return (IOMUX->SECCFG.PINCM[pincm] & IOMUX_PINCM_PF_MASK);
}

/**
 *  @brief  配置 GPIO 引脚功能和复用 GPIO_setConfigAndMux
 *
 *  @param  index       全局引脚索引号
 *  @param  pinConfig   引脚配置值(GPIO_CFG_* 宏组合)
 *  @param  mux         复用功能编码，GPIO_MUX_GPIO 表示通用 GPIO
 *
 *  @return GPIO_STATUS_SUCCESS 成功; GPIO_STATUS_ERROR 失败
 *
 *  本函数是 GPIO 配置的核心实现，完成以下工作:
 *  1. 配置引脚方向(输入/输出)、驱动强度、上下拉、迟滞等电气特性
 *  2. 配置 IOMUX PINCM 寄存器选择引脚复用功能
 *  3. 如需中断，创建端口 HWI 并使能 NVIC 中断
 *  4. 更新配置表记录
 */
int_fast16_t GPIO_setConfigAndMux(
    uint_least8_t index, GPIO_PinConfig pinConfig, uint32_t mux)
{
    if (index == GPIO_INVALID_INDEX) {
        return (GPIO_STATUS_ERROR);
    }

    uintptr_t key;

    GPIO_Regs *port;
    uint32_t portIdx;
    uint32_t pinMask;
    uint32_t pincm;

    HwiP_Handle hwiHandle;
    HwiP_Params hwiParams;

    /* 边界检查 */
    if (index < GPIO_pinLowerBound || index > GPIO_pinUpperBound) {
        /* Configuring out of bounds index */
        return (GPIO_STATUS_ERROR);
    }

    /* 跳过标记为动态配置的引脚 */
    if (pinConfig & GPIO_DO_NOT_CONFIG) {
        /* Return success if the pin will be configured dynamically */
        return (GPIO_STATUS_SUCCESS);
    }

    /* The particular GPIO port can be determined from the index */
    /* 根据索引定位端口基地址和引脚掩码 */
    port    = (GPIO_Regs *) (indexToPortConfig(index).baseAddress);
    pinMask = indexToPortMask(index);

    /* The lower 16 bits incode the IOMUX PINCM, which is device dependent */
    /* 低 8 位编码 IOMUX PINCM 通道号(器件相关) */
    pincm = pinConfig & 0xFF;

    DL_GPIO_INVERSION inversion          = DL_GPIO_INVERSION_DISABLE;
    DL_GPIO_DRIVE_STRENGTH driveStrength = DL_GPIO_DRIVE_STRENGTH_LOW;
    DL_GPIO_RESISTOR resistor            = DL_GPIO_RESISTOR_NONE;
    DL_GPIO_HYSTERESIS hysteresis        = DL_GPIO_HYSTERESIS_DISABLE;
    DL_GPIO_WAKEUP wakeup                = DL_GPIO_WAKEUP_DISABLE;
    DL_GPIO_HIZ hiZ                      = DL_GPIO_HIZ_DISABLE;

    /* Input and output pins support inversion */
    /* 检查是否启用信号反转 */
    if ((pinConfig & GPIO_CFG_INVERSION_MASK) == GPIO_CFG_INVERT_ON) {
        inversion = DL_GPIO_INVERSION_ENABLE;
    }

    /* Input and output pins both support pull-up and pull-down resistors */
    /* 根据配置选择上拉、下拉或无电阻 */
    if (((pinConfig & GPIO_CFG_IO_MASK) == GPIO_CFG_OUT_OD_PU) ||
        ((pinConfig & GPIO_CFG_IO_MASK) == GPIO_CFG_IN_PU)) {
        resistor = DL_GPIO_RESISTOR_PULL_UP;
    } else if (((pinConfig & GPIO_CFG_IO_MASK) == GPIO_CFG_OUT_OD_PD) ||
               ((pinConfig & GPIO_CFG_IO_MASK) == GPIO_CFG_IN_PD)) {
        resistor = DL_GPIO_RESISTOR_PULL_DOWN;
    }

    /*
     * Input pins can set Hi-Z independently. This is mainly for open drain
     * interface purposes such as I2C
     */
    /* 高阻态配置，主要用于 I2C 等开漏接口 */
    if ((pinConfig & GPIO_CFG_HIZ_MASK) == GPIO_CFG_HIZ_ON) {
        hiZ = DL_GPIO_HIZ_ENABLE;
    }

    /* 关中断进入临界区 */
    key = HwiP_disable();

    if (pinConfig & GPIO_CFG_INPUT) {
        /* 输入引脚配置: 迟滞和唤醒功能 */
        if ((pinConfig & GPIO_CFG_HYSTERESIS_MASK) == GPIO_CFG_HYSTERESIS_ON) {
            hysteresis = DL_GPIO_HYSTERESIS_ENABLE;
        }

        if ((pinConfig & GPIO_CFG_WAKEUP_MASK) == GPIO_CFG_WAKEUP_ON_0) {
            wakeup = DL_GPIO_WAKEUP_ON_0;
        } else if ((pinConfig & GPIO_CFG_WAKEUP_MASK) ==
                   GPIO_CFG_WAKEUP_ON_1) {
            wakeup = DL_GPIO_WAKEUP_ON_1;
        }

        /* 初始化外设输入功能，配置 PINCM 寄存器 */
        DL_GPIO_initPeripheralInputFunctionFeatures(
            pincm, mux, inversion, resistor, hysteresis, wakeup);

        /* Check for case where the pins are input but need Hi-Z (I2C) */
        /* 输入引脚启用高阻态(如 I2C 场景) */
        if (hiZ == DL_GPIO_HIZ_ENABLE) {
            DL_GPIO_enableHiZ(pincm);
        }

    } else {
        /* 输出引脚配置: 驱动强度 */
        if ((pinConfig & GPIO_CFG_OUT_STRENGTH_MASK) ==
            GPIO_CFG_OUT_STR_HIGH) {
            driveStrength = DL_GPIO_DRIVE_STRENGTH_HIGH;
        }

        /* 初始化外设输出功能，配置 PINCM 寄存器 */
        DL_GPIO_initPeripheralOutputFunctionFeatures(
            pincm, mux, inversion, resistor, driveStrength, hiZ);

        /* GPIO 模式下使能端口输出 */
        if (mux == GPIO_MUX_GPIO) {
            DL_GPIO_enableOutput(port, pinMask);
        }

        /* Set output pin state */
        /* 设置初始输出电平 */
        if ((pinConfig & GPIO_CFG_OUT_HIGH) == GPIO_CFG_OUT_HIGH) {
            GPIO_write(index, 1);
        } else {
            GPIO_write(index, 0);
        }
    }

    HwiP_restore(key);

    uint32_t portBitMask;

    /* 如果配置了中断，需要为端口创建 HWI */
    if ((pinConfig & GPIO_CFG_INT_MASK) != GPIO_CFG_IN_INT_NONE) {
        portIdx     = indexToPortId(index);
        portBitMask = 1 << portIdx;

        key = HwiP_disable();

        /* 检查该端口的 HWI 是否已创建，避免重复创建 */
        if ((portHwiCreatedBitMask & portBitMask) == 0) {
            portHwiCreatedBitMask |= portBitMask;
            HwiP_restore(key);

            /* 首次为该端口创建 HWI 中断处理 */
            HwiP_Params_init(&hwiParams);

            hwiParams.arg      = (uintptr_t) portIdx;
            hwiParams.priority = GPIO_config.intPriority;

            hwiHandle = HwiP_create(indexToPortConfig(index).interruptNum,
                GPIO_hwiIntFxn, &hwiParams);

            if (hwiHandle == NULL) {
                return (GPIO_STATUS_ERROR);
            }

            /* Can also set hwiParams.enableInt to true */
            /* 使能 NVIC 中断 */
            NVIC_EnableIRQ(indexToPortConfig(index).interruptNum);
        } else {
            HwiP_restore(key);
        }
    }

    /* 配置引脚中断触发方式 */
    GPIO_setInterruptConfig(index, pinConfig);

    key = HwiP_disable();

    /* 更新配置表 */
    GPIO_config.configs[index] = pinConfig;

    HwiP_restore(key);

    return GPIO_STATUS_SUCCESS;
}
