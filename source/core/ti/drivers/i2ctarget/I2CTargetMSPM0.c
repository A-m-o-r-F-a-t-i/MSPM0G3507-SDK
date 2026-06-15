/*
 * Copyright (c) 2023-2024, Texas Instruments Incorporated
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
 *  ======== I2CTargetMSPM0.c ========
 *  I2CTarget MSPM0 平台驱动实现，包含外设初始化、IO 配置、中断处理及开/停/关操作
 */
/* clang-format off */

#include <stdint.h>

#include <ti/drivers/GPIO.h>
#include <ti/drivers/i2ctarget/I2CTargetMSPM0.h>

#ifdef POWER_MANAGEMENT_MSPM0
#include <ti/drivers/Power.h>
#include <ti/drivers/power/PowerMSPM0.h>
#endif

#include <ti/drivers/dpl/HwiP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/DebugP.h>

/* Driverlib header files */
#include <ti/devices/DeviceFamily.h>

/* Prototypes 函数前向声明 */
static void I2CTargetMSPM0_hwiFxn(uintptr_t arg);         /* Hwi 中断处理函数 */
static void I2CTargetMSPM0_configHw(I2CTarget_Handle handle); /* 硬件配置函数 */
static int_fast16_t I2CTargetMSPM0_initIO(I2CTarget_Handle handle); /* IO 初始化函数 */
static void I2CTargetMSPM0_Init(I2CTarget_Handle handle);  /* 外设初始化函数 */

/* Default I2C parameters structure 默认 I2C 参数结构体 */
extern const I2CTarget_Params I2CTarget_defaultParams;

/*!
 *  ======== I2CTargetMSPM0_Init ========
 *  初始化 I2C Target 外设：配置时钟、地址、FIFO 阈值、时钟延展及手动 ACK 模式
 */
static void I2CTargetMSPM0_Init(I2CTarget_Handle handle)
{
    I2CTargetMSPM0_Object *object         = handle->object;
    I2CTargetMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    /* Initialize target mode 初始化 Target 模式 */
    DebugP_assert(I2CBaseValid(hwAttrs->i2c)); /* Check for valid i2c base address 校验 I2C 基地址 */
    DebugP_assert(!((uint8_t)object->targetAddress & 0x80)); /* Check for 7bit address 校验 7 位地址 */

    /* Enable the clock to the target module 使能 Target 模块时钟 */
    DL_I2C_ClockConfig clockConfig =
    {
        .clockSel    = hwAttrs->clockSource,
        .divideRatio = hwAttrs->clockDivider,
    };
    DL_I2C_setClockConfig(hwAttrs->i2c, (DL_I2C_ClockConfig *) &clockConfig);
    /* Set the own target address. only one target address is supported 设置从机自身地址，仅支持单地址 */
    DL_I2C_setTargetOwnAddress(hwAttrs->i2c, object->targetAddress);

    /* Flush the Tx/Rx FIFOs 清空发送和接收 FIFO */
    DL_I2C_flushTargetTXFIFO(hwAttrs->i2c);
    DL_I2C_flushTargetRXFIFO(hwAttrs->i2c);

    /* Set the FIFO threshold 设置 FIFO 中断阈值 */
    DL_I2C_setTargetTXFIFOThreshold(hwAttrs->i2c, hwAttrs->txIntFifoThr);
    DL_I2C_setTargetRXFIFOThreshold(hwAttrs->i2c, hwAttrs->rxIntFifoThr);

    /* Configure the Clock Stretching 配置时钟延展 */
    if (hwAttrs->isClockStretchingEnabled)
    {
        DL_I2C_enableTargetClockStretching(hwAttrs->i2c);
    }

    /* Configure the Analog Glitch Filter 配置模拟毛刺滤波器 */
    if(hwAttrs->isAnalogGlitchFilterEnabled)
    {
        DL_I2C_disableAnalogGlitchFilter(hwAttrs->i2c);
    }
    /* Enable the tx fifo empty interrupt to wait on fifo reload 使能 TX FIFO 空中断以等待重载 */
    DL_I2C_enableTargetTXEmptyOnTXRequest(hwAttrs->i2c);
    /* Disable SWUEN 禁用 Target 唤醒 */
    DL_I2C_disableTargetWakeup(hwAttrs->i2c);
    /* Enable manual ack mode 使能手动 ACK 模式 */
    DL_I2C_setTargetACKOverrideValue(hwAttrs->i2c, 0);
    DL_I2C_enableACKOverrideOnStart(hwAttrs->i2c);

    /* Enable the Target 使能 Target */
    DL_I2C_enableTarget(hwAttrs->i2c);
}

/*!
 *  ======== I2CTarget_open ========
 *  打开 I2C Target 驱动：校验参数、配置 IO、初始化外设硬件、创建 Hwi 和互斥信号量
 */
I2CTarget_Handle I2CTarget_open(uint_least8_t index, I2CTarget_Params *params)
{
    I2CTarget_Handle handle = NULL;
    HwiP_Params hwiParams;
    uintptr_t key;
    I2CTargetMSPM0_Object *object;
    I2CTargetMSPM0_HWAttrs const *hwAttrs;

    /* 校验索引是否在有效范围内 */
    if (index < I2CTarget_count)
    {
        if (params == NULL)
        {
            params = (I2CTarget_Params *)&I2CTarget_defaultParams;
        }
        handle  = (I2CTarget_Handle) & (I2CTarget_config[index]);
        hwAttrs = handle->hwAttrs;
        object  = handle->object;
    }
    else
    {
        return NULL;
    }

    /* Callback function must be defined 回调函数不能为空 */
    if (params->eventCallbackFxn == NULL)
    {
        return NULL;
    }

    /* Only 7-bit address supported by this driver implementation 仅支持 7 位地址 */
    if (params->targetAddress >> 7)
    {
        return NULL;
    }

    /* Determine if the device index was already opened 检查该实例是否已被打开 */
    key = HwiP_disable();
    if (object->isOpen == true)
    {
        HwiP_restore(key);
        return NULL;
    }

    /* Mark the handle as being used 标记句柄为已使用 */
    object->isOpen = true;
    HwiP_restore(key);
#ifdef POWER_MANAGEMENT_MSPM0
    /* Power on the I2C module 上电 I2C 模块 */
    Power_setDependency(hwAttrs->powerMngrId);
#endif
    /* Configure the IOs 配置 I2C 引脚 IO */
    if (I2CTargetMSPM0_initIO(handle) != I2CTarget_STATUS_SUCCESS)
    {
        /* Mark the module as available 打开失败，标记模块为可用 */
        object->isOpen = false;
        #ifdef POWER_MANAGEMENT_MSPM0
        /* Release dependency if open fails 释放电源依赖 */
        Power_releaseDependency(hwAttrs->powerMngrId);
        #endif
        /* Signal back to application that I2C driver was not successfully opened */
        return NULL;
    }

    /* Save parameters 保存参数 */
    object->eventCallbackFxn = params->eventCallbackFxn;
    object->targetAddress    = params->targetAddress;

    /* Set driver state 设置驱动状态为已停止 */
    object->currentState = I2CTarget_State_STOPPED;

    /* Configure HW based on driver state 根据驱动状态配置硬件 */
    I2CTargetMSPM0_configHw(handle);

    /* Create Hwi object for this I2C peripheral 创建 I2C 外设的硬件中断对象 */
    HwiP_Params_init(&hwiParams);
    hwiParams.arg      = (uintptr_t)handle;
    hwiParams.priority = hwAttrs->intPriority;
    HwiP_construct(&(object->hwi), hwAttrs->intNum, I2CTargetMSPM0_hwiFxn, &hwiParams);

    /* Create thread safe handles for this I2C peripheral
     * Semaphore to provide exclusive access to the I2C peripheral
     * 创建互斥信号量，提供对 I2C 外设的独占访问
     */
    SemaphoreP_constructBinary(&(object->mutex), 1);

    /* Return the address of the handle 返回句柄地址 */
    return handle;
}

/*!
 *  @brief  Start I2C Target driver listening on I2C bus.
 *          启动 I2C Target 驱动，在总线上开始监听
 *
 *  @param  handle  A I2CTarget_Handle returned from I2CTarget_open
 *                  I2CTarget_open 返回的驱动句柄
 */
void I2CTarget_start(I2CTarget_Handle handle)
{
    I2CTargetMSPM0_Object *object;

    /* Get the pointer to the object 获取对象指针 */
    object = handle->object;

    /* 设置驱动状态为空闲，准备接收总线事件 */
    object->currentState = I2CTarget_State_IDLE;

    /* Configure HW based on driver state 根据驱动状态配置硬件 */
    I2CTargetMSPM0_configHw(handle);

    #ifdef POWER_MANAGEMENT_MSPM0
    /* Set standby disallow constraint. 设置禁止待机的电源约束 */
    Power_setConstraint(PowerMSPM0_DISALLOW_STANDBY);
    #endif
}

/*!
 *  @brief  Stop I2C Target driver from listening on I2C bus.
 *          停止 I2C Target 驱动，不再监听总线
 *
 *  @param  handle  A I2CTarget_Handle returned from I2CTarget_open
 *                  I2CTarget_open 返回的驱动句柄
 */
void I2CTarget_stop(I2CTarget_Handle handle)
{
    I2CTargetMSPM0_Object *object;

    /* Get the pointer to the object 获取对象指针 */
    object = handle->object;

    /* 设置驱动状态为已停止 */
    object->currentState = I2CTarget_State_STOPPED;

    /* Configure HW based on driver state 根据驱动状态配置硬件 */
    I2CTargetMSPM0_configHw(handle);

    #ifdef POWER_MANAGEMENT_MSPM0
    /* Release power constraint 释放电源约束，允许进入待机 */
    Power_releaseConstraint(PowerMSPM0_DISALLOW_STANDBY);
    #endif
}

/*!
 *  ======== I2CTarget_close ========
 *  关闭 I2C Target 驱动：停止操作、禁用外设、销毁 Hwi 和信号量、释放电源资源
 */
void I2CTarget_close(I2CTarget_Handle handle)
{
    I2CTargetMSPM0_Object *object;
    I2CTargetMSPM0_HWAttrs const *hwAttrs;

    /* Get the pointer to the object and hwAttrs 获取对象和硬件属性指针 */
    hwAttrs = handle->hwAttrs;
    object  = handle->object;

    /* Stop I2C Target operation 若尚未停止则先停止 */
    if (object->currentState != I2CTarget_State_STOPPED)
    {
        I2CTarget_stop(handle);
    }

    /* Disable the I2C Target 禁用 I2C Target */
    DL_I2C_disableTarget(hwAttrs->i2c);

    #ifdef POWER_MANAGEMENT_MSPM0
    /* Power off the I2C module 关闭 I2C 模块电源 */
    Power_releaseDependency(hwAttrs->powerMngrId);
    #endif

    /* Destruct modules used in driver 销毁驱动使用的 Hwi 和互斥信号量 */
    HwiP_destruct(&(object->hwi));
    SemaphoreP_destruct(&(object->mutex));

    #ifdef POWER_MANAGEMENT_MSPM0
    /* Unregister power post notification object 取消电源通知注册 */
    Power_unregisterNotify(&object->i2cPostObj);
    #endif

    /* Mark the module as available 标记模块为可用 */
    object->currentState = I2CTarget_State_STOPPED;
    object->isOpen       = false;

    return;
}

/*
 *  ======== I2CTargetMSPM0_hwiFxn ========
 *  Hwi interrupt handler to service the I2C peripheral
 *  I2C 外设硬件中断处理函数，根据中断标志和状态分发事件到用户回调
 *
 *  The handler is a generic handler for a I2C object.
 */
static void I2CTargetMSPM0_hwiFxn(uintptr_t arg)
{
    I2CTarget_Handle handle              = (I2CTarget_Handle)arg;
    I2CTargetMSPM0_Object *object         = handle->object;
    I2CTargetMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;
    uint8_t dataByte;
    I2CTarget_Event event;

    /* Get interrupt status and clear 获取中断状态并清除标志 */
    uint32_t intFlags = DL_I2C_getEnabledInterruptStatus(hwAttrs->i2c, 0xFFFF0000);
    DL_I2C_clearInterruptStatus(hwAttrs->i2c, I2CTARGET_INT_FLAGS);

    /* Check if controller has written or requested data 获取 Target 状态 */
    uint32_t status = DL_I2C_getTargetStatus(hwAttrs->i2c);

    /* Controller will write to target (target-receive) 主机将写入数据（从机接收） */
    if (status & DL_I2C_TARGET_STATUS_RX_MODE) /* This state will tell us that byte is coming RX 模式表示数据即将到来 */
    {
        object->currentState = I2CTarget_State_RECEIVE;
        object->eventCallbackFxn(handle, I2CTarget_Event_WRITE_REQUESTED, NULL);
    }

    /* Controller has written first byte to target (target-receive) 主机已写入数据字节 */
    if ((status & DL_I2C_TARGET_STATUS_RECEIVE_REQUEST)||(intFlags & DL_I2C_INTERRUPT_TARGET_RX_DONE))
    {
        /* Enable the manual ACK if it is not enabled 若未使能手动 ACK 则启用 */
        if(!DL_I2C_isACKOverrideOnStartEnabled(hwAttrs->i2c))
        {
            DL_I2C_enableACKOverrideOnStart(hwAttrs->i2c);
        }
        /* Read data byte from I2C peripheral 从 I2C 外设读取数据字节 */
        dataByte = DL_I2C_receiveTargetData(hwAttrs->i2c);

        object->currentState = I2CTarget_State_RECEIVE;
        object->eventCallbackFxn(handle, I2CTarget_Event_WRITE_RECEIVED, &dataByte);
        /* Send the ACK 发送 ACK 应答 */
        DL_I2C_setTargetACKOverrideValue(hwAttrs->i2c, 0);
    }

    /* Controller wants to read from target (target-transmit) 主机请求读取数据（从机发送） */
    if ((status & DL_I2C_TARGET_STATUS_TRANSMIT_REQUEST)||(intFlags & DL_I2C_INTERRUPT_TARGET_TXFIFO_EMPTY))
    {
        /* Disable manual ACK so it is handled automatically during transmit 发送时禁用手动 ACK，由硬件自动处理 */
        DL_I2C_disableACKOverrideOnStart(hwAttrs->i2c);

        /* Use internal state machine to differentiate between
         * - the first byte (I2CTarget_Event_READ_REQUESTED), and
         * - subsequent bytes (I2CTarget_Event_READ_PROCESSED).
         * 通过内部状态机区分首个字节（READ_REQUESTED）和后续字节（READ_PROCESSED）
         */
        event = I2CTarget_Event_READ_REQUESTED;
        if (object->currentState == I2CTarget_State_TRANSMIT)
        {
            event = I2CTarget_Event_READ_PROCESSED;
        }

        object->currentState = I2CTarget_State_TRANSMIT;

        /* Call callback function 调用用户回调获取待发送数据 */
        object->eventCallbackFxn(handle, event, &dataByte);

        /* Write byte to I2C peripheral 将数据字节写入 I2C 外设 */
        DL_I2C_transmitTargetData(hwAttrs->i2c, dataByte);
    }

    /* Stop condition received 收到停止条件 */
    if (intFlags & DL_I2C_INTERRUPT_TARGET_STOP)
    {
        object->currentState = I2CTarget_State_IDLE;
        object->eventCallbackFxn(handle, I2CTarget_Event_STOP, NULL);
    }
}

/*
 *  ======== I2CTargetMSPM0_configHw ========
 *  This functions configures the I2C hardware module.
 *  根据驱动状态配置 I2C 硬件模块：停止态禁用中断和 Target，运行态使能中断并初始化
 *
 *  @pre    Function assumes that the I2C handle is pointing to a hardware
 *          module which has already been opened.
 *          假设 I2C 句柄指向已打开的硬件模块
 */
static void I2CTargetMSPM0_configHw(I2CTarget_Handle handle)
{
    I2CTargetMSPM0_Object *object         = handle->object;
    I2CTargetMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    if (object->currentState == I2CTarget_State_STOPPED)
    {
        /* Driver is stopped/inactive. Disable interrupts 驱动已停止，禁用中断 */
        DL_I2C_disableInterrupt(hwAttrs->i2c, I2CTARGET_INT_FLAGS);

        /* Disable I2C module target mode 禁用 I2C Target 模式 */
        DL_I2C_disableTarget(hwAttrs->i2c);

        /* Clear interrupt flags 清除中断标志 */
        DL_I2C_clearInterruptStatus(hwAttrs->i2c, I2CTARGET_INT_FLAGS);
    }
    else
    {
        /* Driver is started/active. Clear flags and enable interrupts. 驱动运行中，清除标志并使能中断 */
        DL_I2C_clearInterruptStatus(hwAttrs->i2c, I2CTARGET_INT_FLAGS);
        DL_I2C_enableInterrupt(hwAttrs->i2c, I2CTARGET_INT_FLAGS);

        /* Initialize the I2C 初始化 I2C 外设 */
        I2CTargetMSPM0_Init(handle);
    }
}

/*
 *  ======== I2CTargetMSPM0_initIO ========
 *  This functions initializes the I2C IOs.
 *  初始化 I2C 引脚 IO：配置 SDA/SCL 为开漏上拉输入、高电平输出、高阻态，并设置引脚复用
 *
 *  @pre    Function assumes that the I2C handle is pointing to a hardware
 *          module which has already been opened.
 *          假设 I2C 句柄指向已打开的硬件模块
 */
static int_fast16_t I2CTargetMSPM0_initIO(I2CTarget_Handle handle)
{
    I2CTargetMSPM0_HWAttrs const *hwAttrs;

    /* Get the pointer to hwAttrs 获取硬件属性指针 */
    hwAttrs = handle->hwAttrs;

    /* Configure I2C pins SDA and SCL and set their muxes 配置 SDA 和 SCL 引脚并设置复用 */
    GPIO_setConfigAndMux(hwAttrs->sdaPinIndex,GPIO_CFG_INPUT | GPIO_CFG_IN_PU | GPIO_CFG_OUT_HIGH | GPIO_CFG_HIZ_ON | hwAttrs->sdaPincm,hwAttrs->sdaPinMux);
    GPIO_setConfigAndMux(hwAttrs->sclPinIndex,GPIO_CFG_INPUT | GPIO_CFG_IN_PU | GPIO_CFG_OUT_HIGH | GPIO_CFG_HIZ_ON | hwAttrs->sclPincm,hwAttrs->sclPinMux);

    return I2CTarget_STATUS_SUCCESS;
}
/* clang-format on */
