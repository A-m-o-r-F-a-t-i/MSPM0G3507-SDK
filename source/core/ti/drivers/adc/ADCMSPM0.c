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
/*
 * ADCMSPM0.c - MSPM0 平台 ADC 驱动实现
 * 提供 ADC 初始化、打开、关闭、单通道转换、链式转换及电压换算等功能。
 */
/* clang-format off */

#include <stdint.h>

/* Kernel services */
#include <ti/drivers/dpl/DebugP.h>
#include <ti/drivers/dpl/SemaphoreP.h>
#include <ti/drivers/dpl/HwiP.h>

/* TI drivers */
#include <ti/drivers/ADC.h>
#include <ti/drivers/adc/ADCMSPM0.h>
#include <ti/drivers/GPIO.h>
#include <ti/drivers/gpio/GPIOMSPM0.h>
#include <ti/driverlib/m0p/dl_core.h>

#include <ti/devices/DeviceFamily.h>

/*
 * =============================================================================
 * Public Function Declarations
 * =============================================================================
 */
void ADCMSPM0_close(ADC_Handle handle);
void ADCMSPM0_init(ADC_Handle handle);
ADC_Handle ADCMSPM0_open(ADC_Handle handle, ADC_Params *params);
int_fast16_t ADCMSPM0_convert(ADC_Handle handle, uint16_t *value);
int_fast16_t ADCMSPM0_convertChain(ADC_Handle *handleList, uint16_t *dataBuffer, uint8_t channelCount);
int_fast16_t ADCMSPM0_control(ADC_Handle handle, uint_fast16_t cmd, void *arg);
uint32_t ADCMSPM0_convertToMicroVolts(ADC_Handle handle, uint16_t adcValue);

/*
 * =============================================================================
 * Private Function Declarations
 * =============================================================================
 */
static void ADCMSPM0_initHw(ADC_Handle handle, ADC_Params *params);
static uint32_t ADCMSPM0_ADCValueToMicrovolts(uint32_t adcCode, uint32_t bitResolution, uint32_t referenceVoltageMicroVolt);
/*
 * =============================================================================
 * Constants
 * =============================================================================
 */

/* ADC function table for ADCMSPM0 implementation */
const ADC_FxnTable ADCMSPM0_fxnTable = {ADCMSPM0_close,
                                       ADCMSPM0_control,
                                       ADCMSPM0_convert,
                                       ADCMSPM0_convertChain,
                                       ADCMSPM0_convertToMicroVolts,
                                       ADCMSPM0_init,
                                       ADCMSPM0_open};

/*
 * =============================================================================
 * Private Global Variables
 * =============================================================================
 */

/* Keep track of the adc handle instance to create and delete adcSemaphore */
static uint16_t adcInstance = 0;

/* Semaphore to arbitrate access to the single ADC peripheral between multiple handles */
static SemaphoreP_Struct adcSemaphore;

/*
 * =============================================================================
 * Function Definitions
 * =============================================================================
 */

/**
 * @brief 初始化 ADC 驱动实例 / Initialize ADC driver instance
 * @param handle ADC 句柄
 */
void ADCMSPM0_init(ADC_Handle handle)
{
    ADCMSPM0_Object *object;

    /* Get the object / 获取对象指针 */
    object = handle->object;

    /* Mark the object as available / 标记对象为可用状态 */
    object->isOpen = false;
}

/**
 * @brief 打开 ADC 外设并完成初始化 / Open ADC peripheral and initialize
 * @param handle ADC 句柄
 * @param params ADC 参数
 * @return 成功返回句柄，失败返回 NULL
 */
ADC_Handle ADCMSPM0_open(ADC_Handle handle, ADC_Params *params)
{
    ADCMSPM0_Object *object;
    ADCMSPM0_HWAttrs const *hwAttrs;

    DebugP_assert(handle);

    /* Get object and hwAttrs / 获取对象和硬件属性 */
    object  = handle->object;
    hwAttrs = handle->hwAttrs;

    /* Determine if the driver was already opened / 检查驱动是否已打开 */
    uint32_t key = HwiP_disable();

    if (object->isOpen)
    {
        DebugP_log0("ADC: Error! Already in use.");
        HwiP_restore(key);
        return NULL;
    }
    object->isOpen = true;

    /* Remember thread safety protection setting / 记录线程安全保护设置 */
    object->isProtected = params->isProtected;

    /* If this is the first handle requested, set up the semaphore as well / 首次打开时创建信号量 */
    if (adcInstance == 0)
    {
        /* Setup semaphore / 创建二值信号量 */
        SemaphoreP_constructBinary(&adcSemaphore, 1);
    }
    adcInstance++;
#ifdef POWER_MANAGEMENT_MSPM0
    /* Register power dependency - i.e. power up and enable clock for ADC / 注册电源依赖并使能时钟 */
    if(hwAttrs->adc == ADC0)
        Power_setDependency(PowerMSPM0_PERIPH_ADC0);
    else if(hwAttrs->adc == ADC1)
        Power_setDependency(PowerMSPM0_PERIPH_ADC1);
#endif

    ADCMSPM0_initHw(handle, params);
    HwiP_restore(key);

    /* Set pins to analog function. If pin is unused, value is set to GPIO_INVALID_INDEX / 配置引脚为模拟功能 */
    GPIO_setConfigAndMux(hwAttrs->adcInputDIO, GPIO_CFG_INPUT | hwAttrs->adcInputPincm, hwAttrs->adcInputPinMux);
    GPIO_setConfigAndMux(hwAttrs->adcPosRefDIO, GPIO_CFG_INPUT | hwAttrs->adcPosRefPincm, hwAttrs->adcPosRefPinMux);
    GPIO_setConfigAndMux(hwAttrs->adcNegRefDIO, GPIO_CFG_INPUT | hwAttrs->adcNegRefPincm, hwAttrs->adcNegRefPinMux);

    DebugP_log0("ADC: Object opened");

    return handle;
}

/**
 * @brief 初始化 ADC 硬件模块 / Initialize ADC hardware module
 * @param handle ADC 句柄
 * @param params ADC 参数
 */
static void ADCMSPM0_initHw(ADC_Handle handle, ADC_Params *params)
{
    ADCMSPM0_Object *object         = handle->object;
    ADCMSPM0_HWAttrs const *hwAttrs = handle->hwAttrs;

    /* 配置 ADC 时钟 / Configure ADC clock */
    DL_ADC12_ClockConfig clockConfig =
    {
        .clockSel    = (DL_ADC12_CLOCK)hwAttrs->adcClkSelect,
        .divideRatio = (DL_ADC12_CLOCK_DIVIDE)hwAttrs->adcClkkDivider,
        .freqRange   = (DL_ADC12_CLOCK_FREQ_RANGE)hwAttrs->adcClkFreqRange,
    };
    DL_ADC12_setClockConfig(hwAttrs->adc, (DL_ADC12_ClockConfig *) &clockConfig);

    /* 配置转换存储器 / Configure conversion memory */
    DL_ADC12_configConversionMem(hwAttrs->adc, hwAttrs->idxMEMCTLx, /*! idxMEMCTLx should match the conversionStartAddr */
            hwAttrs->adcChannel, hwAttrs->refSource, hwAttrs->sampleTimerSrc, hwAttrs->adcHWAveraging,
            DL_ADC12_BURN_OUT_SOURCE_DISABLED, hwAttrs->conversionTrigger, DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    /* Single conversion / 单通道单次转换 */
    if(hwAttrs->conversionMode == ADCMSPM0_SINGLE_CH_SINGLE_CONV)
    {
        DL_ADC12_initSingleSample(hwAttrs->adc,
                                  hwAttrs->repeatConversionMode, hwAttrs->samplingMode, hwAttrs->sampleTrigger,
                                  hwAttrs->resolutionBits, hwAttrs->conversionDataFormat);
    }
    else /* Sequence Conversion / 多通道序列转换 */
    {
        /* The below function is configured for single conversion */
        DL_ADC12_initSeqSample(hwAttrs->adc,
            hwAttrs->repeatConversionMode, hwAttrs->samplingMode, hwAttrs->sampleTrigger,
            hwAttrs->conversionStartAddr, hwAttrs->conversionEndAddr, hwAttrs->resolutionBits,
            hwAttrs->conversionDataFormat);
    }
    /* 设置手动掉电模式 / Set manual power down mode */
    DL_ADC12_setPowerDownMode(hwAttrs->adc,ADCMSPM0_Pwr_Dn_MODE_MANUAL);
    /* 配置采样时间 / Configure sample time */
    if(hwAttrs->sampleTimerSrc == ADCMSPM0_SAMP_TMR_SOURCE_SCOMP0)
    {
        DL_ADC12_setSampleTime0(hwAttrs->adc,hwAttrs->samplingDuration);
    }
    else if(hwAttrs->sampleTimerSrc == ADCMSPM0_SAMP_TMR_SOURCE_SCOMP1)
    {
        DL_ADC12_setSampleTime1(hwAttrs->adc,hwAttrs->samplingDuration);
    }
    /* If hardware averaging is enabled then configure it.
    *  The hardware averaging is configured if it is enabled for any of the channel.
    *  The configuration is common for all the channel but it can be enabled or disabled
    *  for each channel.
    *  若使能硬件平均则进行配置，该配置对所有通道通用，但可按通道单独开关。
    */
    if(hwAttrs->adcHWAveraging)
        DL_ADC12_configHwAverage(hwAttrs->adc,hwAttrs->adcHwAvgNumerator, hwAttrs->adcHwAvgDenominator);
    /* Enable the ADC for the operation / 使能 ADC 转换 */
    DL_ADC12_enableConversions(hwAttrs->adc);
}


/**
 * @brief 关闭 ADC 实例 / Close ADC instance
 * @param handle ADC 句柄
 */
void ADCMSPM0_close(ADC_Handle handle)
{
    ADCMSPM0_Object *object;

    DebugP_assert(handle);

    object = handle->object;

    uint32_t key = HwiP_disable();

    if (object->isOpen)
    {
        adcInstance--;
        /* 最后一个实例关闭时销毁信号量 / Destroy semaphore when last instance closes */
        if (adcInstance == 0)
        {
            SemaphoreP_destruct(&adcSemaphore);
        }
#ifdef POWER_MANAGEMENT_MSPM0
        /* Remove power dependency / 释放电源依赖 */
        if(hwAttrs->adc == ADC0)
            Power_releaseDependency(PowerMSPM0_PERIPH_ADC0);
        else if(hwAttrs->adc == ADC1)
            Power_releaseDependency(PowerMSPM0_PERIPH_ADC1);
#endif
        DebugP_log0("ADC: Object closed");
    }

    object->isOpen = false;
    HwiP_restore(key);
}

/**
 * @brief ADC 控制命令（暂未实现）/ ADC control command (not implemented)
 * @param handle ADC 句柄
 * @param cmd 控制命令
 * @param arg 命令参数
 * @return ADC_STATUS_UNDEFINEDCMD
 */
int_fast16_t ADCMSPM0_control(ADC_Handle handle, uint_fast16_t cmd, void *arg)
{
    /* No implementation yet / 暂未实现 */
    return ADC_STATUS_UNDEFINEDCMD;
}

/**
 * @brief 执行单次 ADC 转换 / Perform a single ADC conversion
 * @param handle ADC 句柄
 * @param value 转换结果存放指针
 * @return ADC_STATUS_SUCCESS 或 ADC_STATUS_ERROR
 */
int_fast16_t ADCMSPM0_convert(ADC_Handle handle, uint16_t *value)
{
    ADCMSPM0_HWAttrs const *hwAttrs;
    ADCMSPM0_Object *object;
    int_fast16_t conversionResult = ADC_STATUS_SUCCESS;
    uint32_t conversionValue;
    uint32_t interruptStatus;

    DebugP_assert(handle);

    /* Get handle / 获取硬件属性 */
    hwAttrs = handle->hwAttrs;

    /* Get the object / 获取对象 */
    object = handle->object;

    if (object->isProtected)
    {
        /* Acquire the lock for this particular ADC handle / 获取信号量锁 */
        SemaphoreP_pend(&adcSemaphore, SemaphoreP_WAIT_FOREVER);
    }
#ifdef POWER_MANAGEMENT_MSPM0
    /* Set constraints to guarantee operation / 禁止进入待机以保证转换完成 */
    Power_setConstraint(PowerMSPM0_DISALLOW_STANDBY);
#endif

    /* Trigger a conversion / 启动转换 */
    DL_ADC12_startConversion(hwAttrs->adc);

    /* Read out conversion (blocking while ADC is busy) / 阻塞等待转换完成 */
    while(!(DL_ADC12_getRawInterruptStatus(hwAttrs->adc,(DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED<<hwAttrs->idxMEMCTLx))));
    conversionValue = DL_ADC12_getMemResult(hwAttrs->adc, hwAttrs->idxMEMCTLx);

    /* Check if something went wrong. (Underflow or overflow) / 检查溢出/下溢错误 */
    interruptStatus = DL_ADC12_getRawInterruptStatus(hwAttrs->adc, ADC_ALL_INTERRUPTS_MASK);

    if (interruptStatus & (DL_ADC12_INTERRUPT_OVERFLOW | DL_ADC12_INTERRUPT_UNDERFLOW))
    {
        conversionResult = ADC_STATUS_ERROR;
    }

    /* Clear all interrupts generated by ADC / 清除所有 ADC 中断标志 */
    DL_ADC12_clearInterruptStatus(hwAttrs->adc, ADC_ALL_INTERRUPTS_MASK);

#ifdef POWER_MANAGEMENT_MSPM0
    /* Allow entering standby again after ADC conversion complete / 转换完成后允许待机 */
    Power_releaseConstraint(PowerMSPM0_DISALLOW_STANDBY);
#endif

    if (object->isProtected)
    {
        /* Release the lock for this particular ADC handle / 释放信号量锁 */
        SemaphoreP_post(&adcSemaphore);
    }

    *value = conversionValue;

    /* Return the status-code of the conversion / 返回转换状态 */
    return conversionResult;
}

/**
 * @brief 执行链式多通道 ADC 转换 / Perform chained multi-channel ADC conversion
 * @param handleList ADC 句柄数组
 * @param dataBuffer 转换结果缓冲区
 * @param channelCount 通道数量
 * @return ADC_STATUS_SUCCESS 或 ADC_STATUS_ERROR
 */
int_fast16_t ADCMSPM0_convertChain(ADC_Handle *handleList, uint16_t *dataBuffer, uint8_t channelCount)
{
    ADCMSPM0_HWAttrs const *hwAttrs;
    ADCMSPM0_Object *object;
    int_fast16_t conversionResult = ADC_STATUS_SUCCESS;
    uint32_t conversionValue;
    uint32_t interruptStatus;

    /* Acquire the lock used arbitrate access to the ADC peripheral
     * between multiple handles.
     * 获取信号量锁以仲裁多句柄对 ADC 外设的访问
     */
    SemaphoreP_pend(&adcSemaphore, SemaphoreP_WAIT_FOREVER);
#ifdef POWER_MANAGEMENT_MSPM0
    /* Set constraints to guarantee operation / 禁止进入待机 */
    Power_setConstraint(PowerMSPM0_DISALLOW_STANDBY);
#endif

    /* 逐通道执行转换 / Convert each channel sequentially */
    for (uint32_t i = 0; i < channelCount; i++)
    {
        DebugP_assert(handleList[i]);

        /* Get the particular handle / 获取当前通道句柄 */
        hwAttrs = handleList[i]->hwAttrs;
        object = handleList[i]->object;

        /* Trigger a conversion / 启动转换 */
        DL_ADC12_startConversion(hwAttrs->adc);

        /* Read out conversion (blocking while ADC is busy) / 阻塞等待转换完成 */
        while(!(DL_ADC12_getRawInterruptStatus(hwAttrs->adc,(DL_ADC12_INTERRUPT_MEM0_RESULT_LOADED<<hwAttrs->idxMEMCTLx))));
        conversionValue = DL_ADC12_getMemResult(hwAttrs->adc, hwAttrs->idxMEMCTLx);

        dataBuffer[i] = conversionValue;
    }

    /* Check if something went wrong. (Underflow or overflow) / 检查溢出/下溢 */
    interruptStatus = DL_ADC12_getRawInterruptStatus(hwAttrs->adc, ADC_ALL_INTERRUPTS_MASK);

    if (interruptStatus & (DL_ADC12_INTERRUPT_OVERFLOW | DL_ADC12_INTERRUPT_UNDERFLOW))
    {
        conversionResult = ADC_STATUS_ERROR;
    }

    /* Clear all interrupts generated by ADC / 清除所有 ADC 中断 */
    DL_ADC12_clearInterruptStatus(hwAttrs->adc, ADC_ALL_INTERRUPTS_MASK);

#ifdef POWER_MANAGEMENT_MSPM0
    /* Allow entering standby again after ADC conversion complete / 允许待机 */
    Power_releaseConstraint(PowerMSPM0_DISALLOW_STANDBY);
#endif
    /* Release the lock used arbitrate access to the single ADC peripheral
     * between multiple handles.
     * 释放信号量锁
     */
    SemaphoreP_post(&adcSemaphore);

    /* Return the status-code of the conversion / 返回转换状态 */
    return conversionResult;
}

/**
 * @brief 将 ADC 原始值转换为微伏 / Convert ADC raw value to microvolts
 * @param handle ADC 句柄
 * @param adcValue ADC 原始转换值
 * @return 电压值(微伏)
 */
uint32_t ADCMSPM0_convertToMicroVolts(ADC_Handle handle, uint16_t adcValue)
{
    ADCMSPM0_HWAttrs const *hwAttrs;
    uint32_t microVolts;

    DebugP_assert(handle);

    /* Get the pointer to the hwAttrs / 获取硬件属性指针 */
    hwAttrs = handle->hwAttrs;

    microVolts = ADCMSPM0_ADCValueToMicrovolts((uint32_t)adcValue, hwAttrs->resolutionBits, hwAttrs->refVoltage);

    return microVolts;
}

/**
 * @brief ADC 原始码值转微伏（内部实现）/ Convert ADC code to microvolts (internal)
 * @param adcCode ADC 原始码值
 * @param bitResolution 分辨率(8/10/12 位)
 * @param referenceVoltageMicroVolt 参考电压(微伏)
 * @return 电压值(微伏)
 */
static uint32_t ADCMSPM0_ADCValueToMicrovolts(uint32_t adcCode, uint32_t bitResolution, uint32_t referenceVoltageMicroVolt)
{
    uint32_t adcMaxCode;
    uint32_t shift;
    uint32_t microVolts;

    /* 根据分辨率确定最大码值和移位量 / Determine max code and shift by resolution */
    if (bitResolution == ADCMSPM0_RESOLUTION_8_BIT)
    {
        adcMaxCode = 0xFF;
        shift      = 0;
    }
    else if (bitResolution == ADCMSPM0_RESOLUTION_10_BIT)
    {
        adcMaxCode = 0x3FF;
        shift      = 2;
    }
    else /* Default to 12-bit / 默认 12 位 */
    {
        adcMaxCode = 0xFFF;
        shift      = 4;
    }

    /* shift down voltage to avoid 32bit overflow / 右移参考电压防止 32 位溢出 */
    referenceVoltageMicroVolt >>= shift;

    /* Convert from code to microvolts / 码值换算为微伏，含四舍五入 */
    microVolts = (((adcCode * referenceVoltageMicroVolt) + (adcMaxCode >> 1)) / adcMaxCode);

    /* Shift result back up / 左移还原结果 */
    microVolts <<= shift;

    return microVolts;
}

/* clang-format on */
