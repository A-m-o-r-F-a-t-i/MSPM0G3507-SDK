/*
* Copyright (c) 2020, Texas Instruments Incorporated
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
/*!****************************************************************************
*  @file       dl_timerg.h
*  @brief      General Purpose Timer G (TIMG) Driver Library
*              通用定时器 G (TIMG) 驱动库
*  @defgroup   TIMG General Purpose Timer G (TIMG)
*
*  @anchor ti_dl_dl_timerg__Overview
*  # Overview
*
*   The TimerG Driver Library allows you to configure the General Purpose Timer
*   (TIMG) module in output compare, input capture, PWM output, one-shot and
*   periodic modes. For detailed TIMG features please refer to the Device
*   Technical Reference Manual (TRM)
*
*  <hr>
******************************************************************************
*/
/** @addtogroup TIMG
 * @{
 */
#ifndef ti_dl_dl_timerg__include
#define ti_dl_dl_timerg__include

#include <stdbool.h>
#include <stdint.h>

#include <ti/driverlib/dl_timer.h>

#ifdef __MSPM0_HAS_TIMER_G__

#ifdef __cplusplus
extern "C" {
#endif

/* clang-format off */

/**
 * @brief Redirects to common @ref DL_Timer_backupConfig
 *        配置备份结构体别名
 */
typedef DL_Timer_backupConfig                           DL_TimerG_backupConfig;


/**
 * @brief Redirects to common @ref DL_Timer_ClockConfig
 *        时钟配置结构体别名
 */
typedef DL_Timer_ClockConfig                             DL_TimerG_ClockConfig;

/**
 * @brief Redirects to common @ref DL_Timer_TimerConfig
 *        定时器配置结构体别名
 */
typedef DL_Timer_TimerConfig                             DL_TimerG_TimerConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CaptureConfig
 *        捕获配置结构体别名
 */
typedef DL_Timer_CaptureConfig                         DL_TimerG_CaptureConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CaptureTriggerConfig
 *        捕获触发配置结构体别名
 */
typedef DL_Timer_CaptureTriggerConfig            DL_TimerG_CaptureTriggerConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CaptureCombinedConfig
 *        组合捕获配置结构体别名
 */
typedef DL_Timer_CaptureCombinedConfig          DL_TimerG_CaptureCombinedConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CompareConfig
 *        比较配置结构体别名
 */
typedef DL_Timer_CompareConfig                          DL_TimerG_CompareConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CompareTriggerConfig
 *        比较触发配置结构体别名
 */
typedef DL_Timer_CompareTriggerConfig            DL_TimerG_CompareTriggerConfig;

/**
 * @brief Redirects to common @ref DL_Timer_PWMConfig
 *        PWM 配置结构体别名
 */
typedef DL_Timer_PWMConfig                                  DL_TimerG_PWMConfig;

/** @addtogroup DL_TIMERG_CAPTURE_COMPARE_INDEX
 *  @{
 */

/**
 * @brief  Index associated to Capture Compare 0
 *         捕获比较通道 0 索引
 */
#define DL_TIMERG_CAPTURE_COMPARE_0_INDEX       DL_TIMER_CC_0_INDEX

/**
 * @brief  Index associated to Capture Compare 1
 *         捕获比较通道 1 索引
 */
#define DL_TIMERG_CAPTURE_COMPARE_1_INDEX       DL_TIMER_CC_1_INDEX

/**
 * @brief  Index associated to Capture Compare 2
 *         捕获比较通道 2 索引
 * @note   <b>This option is not supported by all device TimerG modules </b>
 *         please refer to the "TIMx Configurations" table in the
 *         device specific datasheet to determine Timer instances which
 *         support "4 CC Channel" configuration.
 */
#define DL_TIMERG_CAPTURE_COMPARE_2_INDEX       DL_TIMER_CC_2_INDEX

/**
 * @brief  Index associated to Capture Compare 3
 *         捕获比较通道 3 索引
 * @note   <b>This option is not supported by all device TimerG modules </b>
 *         please refer to the "TIMx Configurations" table in the
 *         device specific datasheet to determine Timer instances which
 *         support "4 CC Channel" configuration.
 */
#define DL_TIMERG_CAPTURE_COMPARE_3_INDEX       DL_TIMER_CC_3_INDEX

/** @}*/

/** @addtogroup DL_TIMERG_INTERRUPT
 *  @{
 */
/*!
 * @brief Redirects to common @ref DL_TIMER_INTERRUPT_ZERO_EVENT
 *        计数器归零中断事件
 */
#define DL_TIMERG_INTERRUPT_ZERO_EVENT           (DL_TIMER_INTERRUPT_ZERO_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_LOAD_EVENT
*        计数器装载中断事件
*/
#define DL_TIMERG_INTERRUPT_LOAD_EVENT           (DL_TIMER_INTERRUPT_LOAD_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC0_DN_EVENT
*        CC0 向下计数中断事件
*/
#define DL_TIMERG_INTERRUPT_CC0_DN_EVENT       (DL_TIMER_INTERRUPT_CC0_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC1_DN_EVENT
*        CC1 向下计数中断事件
*/
#define DL_TIMERG_INTERRUPT_CC1_DN_EVENT       (DL_TIMER_INTERRUPT_CC1_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC0_UP_EVENT
*        CC0 向上计数中断事件
*/
#define DL_TIMERG_INTERRUPT_CC0_UP_EVENT       (DL_TIMER_INTERRUPT_CC0_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC1_UP_EVENT
*        CC1 向上计数中断事件
*/
#define DL_TIMERG_INTERRUPT_CC1_UP_EVENT       (DL_TIMER_INTERRUPT_CC1_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC2_DN_EVENT
* @note <b> This interrupt is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_INTERRUPT_CC2_DN_EVENT       (DL_TIMER_INTERRUPT_CC2_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC3_DN_EVENT
* @note <b> This interrupt is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_INTERRUPT_CC3_DN_EVENT       (DL_TIMER_INTERRUPT_CC3_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC2_UP_EVENT
* @note <b> This interrupt is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_INTERRUPT_CC2_UP_EVENT       (DL_TIMER_INTERRUPT_CC2_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC3_UP_EVENT
* @note <b> This interrupt is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_INTERRUPT_CC3_UP_EVENT       (DL_TIMER_INTERRUPT_CC3_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_OVERFLOW_EVENT
*        计数器溢出中断事件
*/
#define DL_TIMERG_INTERRUPT_OVERFLOW_EVENT   (DL_TIMER_INTERRUPT_OVERFLOW_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_DC_EVENT
*        方向变化中断事件
* @note <b> This interrupt is only available on some TimerG variants with
*       Quadrature Encoding Input (QEI). Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_INTERRUPT_DC_EVENT               (DL_TIMER_INTERRUPT_DC_EVENT)


/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_QEIERR_EVENT
*        QEI 错误中断事件
* @note <b> This interrupt is only available on some TimerG variants with
*       Quadrature Encoding Input (QEI). Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_INTERRUPT_QEI_ERR_EVENT      (DL_TIMER_INTERRUPT_QEIERR_EVENT)


/** @}*/

/** @addtogroup DL_TIMERG_EVENT
 *  @{
 */
/*!
 * @brief Redirects to common @ref DL_TIMER_EVENT_ZERO_EVENT
 *        计数器归零事件
 */
#define DL_TIMERG_EVENT_ZERO_EVENT                   (DL_TIMER_EVENT_ZERO_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_LOAD_EVENT
*        计数器装载事件
*/
#define DL_TIMERG_EVENT_LOAD_EVENT                   (DL_TIMER_EVENT_LOAD_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC0_DN_EVENT
*/
#define DL_TIMERG_EVENT_CC0_DN_EVENT               (DL_TIMER_EVENT_CC0_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC1_DN_EVENT
*/
#define DL_TIMERG_EVENT_CC1_DN_EVENT               (DL_TIMER_EVENT_CC1_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC0_UP_EVENT
*/
#define DL_TIMERG_EVENT_CC0_UP_EVENT               (DL_TIMER_EVENT_CC0_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC1_UP_EVENT
*/
#define DL_TIMERG_EVENT_CC1_UP_EVENT               (DL_TIMER_EVENT_CC1_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC2_DN_EVENT
* @note <b> This event is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_EVENT_CC2_DN_EVENT               (DL_TIMER_EVENT_CC2_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC3_DN_EVENT
* @note <b> This event is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_EVENT_CC3_DN_EVENT               (DL_TIMER_EVENT_CC3_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC2_UP_EVENT
* @note <b> This event is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_EVENT_CC2_UP_EVENT               (DL_TIMER_EVENT_CC2_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC3_UP_EVENT
* @note <b> This event is only available on some TimerG variants with
*       4 external CCP Channels. Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_EVENT_CC3_UP_EVENT               (DL_TIMER_EVENT_CC3_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_OVERFLOW_EVENT
*        计数器溢出事件
*/
#define DL_TIMERG_EVENT_OVERFLOW_EVENT           (DL_TIMER_EVENT_OVERFLOW_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_DC_EVENT
*        方向变化事件
* @note <b> This interrupt is only available on some TimerG variants with
*       Quadrature Encoding Input (QEI). Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_EVENT_DC_EVENT                       (DL_TIMER_EVENT_DC_EVENT)


/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_QEIERR_EVENT
*        QEI 错误事件
* @note <b> This interrupt is only available on some TimerG variants with
*       Quadrature Encoding Input (QEI). Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_EVENT_QEIERR_EVENT               (DL_TIMER_EVENT_QEIERR_EVENT)


/** @}*/

/** @addtogroup DL_TIMERG_IIDX
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        无待处理中断
 */
#define DL_TIMERG_IIDX_ZERO                                   DL_TIMER_IIDX_ZERO

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        装载中断索引
 */
#define DL_TIMERG_IIDX_LOAD                                   DL_TIMER_IIDX_LOAD

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        CC0 向下计数中断索引
 */
#define DL_TIMERG_IIDX_CC0_DN                               DL_TIMER_IIDX_CC0_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        CC1 向下计数中断索引
 */
#define DL_TIMERG_IIDX_CC1_DN                               DL_TIMER_IIDX_CC1_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC2_DN
 */
#define DL_TIMERG_IIDX_CC2_DN                               DL_TIMER_IIDX_CC2_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC3_DN
 */
#define DL_TIMERG_IIDX_CC3_DN                               DL_TIMER_IIDX_CC3_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        CC0 向上计数中断索引
 */
#define DL_TIMERG_IIDX_CC0_UP                               DL_TIMER_IIDX_CC0_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        CC1 向上计数中断索引
 */
#define DL_TIMERG_IIDX_CC1_UP                               DL_TIMER_IIDX_CC1_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC2_UP
 */
#define DL_TIMERG_IIDX_CC2_UP                               DL_TIMER_IIDX_CC2_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC3_UP
 */
#define DL_TIMERG_IIDX_CC3_UP                               DL_TIMER_IIDX_CC3_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 *        溢出中断索引
 */
#define DL_TIMERG_IIDX_OVERFLOW                           DL_TIMER_IIDX_OVERFLOW

/**
  * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
  *        方向变化中断索引
  * @note <b> This is only available on some instances of TimerG. Consult the
  *       TRM to determin instances with support this functionality.</b>
  */
#define DL_TIMERG_IIDX_DIR_CHANGE                       DL_TIMER_IIDX_DIR_CHANGE


/*!
* @brief QEI incorrect transition state error
*        QEI 错误转换状态中断索引
* @note <b> This interrupt is only available on some TimerG variants with
*       Quadrature Encoding Input (QEI). Please refer to "TIMx
*       Configurations" table in the device specific datasheet.</b>
*/
#define DL_TIMERG_IIDX_QEIERR                           DL_TIMER_IDX_STAT_QEIERR

/** @}*/

/** @addtogroup DL_TIMERG_PUBLISHER_INDEX
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_PUBLISHER_INDEX_0
 */
#define DL_TIMERG_PUBLISHER_INDEX_0                   DL_TIMER_PUBLISHER_INDEX_0

/**
 * @brief Redirects to common @ref DL_TIMER_PUBLISHER_INDEX_1
 */
#define DL_TIMERG_PUBLISHER_INDEX_1                   DL_TIMER_PUBLISHER_INDEX_1

/** @}*/

/**
 * @brief Redirects to common @ref DL_TIMER_QEI_DIRECTION
 *        QEI 方向枚举别名
 */
#define DL_TIMERG_QEI_DIRECTION                       DL_TIMER_QEI_DIRECTION

/**
 * @brief Redirects to common @ref DL_TIMER_QEI_MODE
 *        QEI 模式枚举别名
 */
#define DL_TIMERG_QEI_MODE                       DL_TIMER_QEI_MODE

/** @addtogroup DL_TIMERG_SUBSCRIBER_INDEX
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_SUBSCRIBER_INDEX_0
 */
#define DL_TIMERG_SUBSCRIBER_INDEX_0                 DL_TIMER_SUBSCRIBER_INDEX_0
/**
 * @brief Redirects to common @ref DL_TIMER_SUBSCRIBER_INDEX_1
 */
#define DL_TIMERG_SUBSCRIBER_INDEX_1                 DL_TIMER_SUBSCRIBER_INDEX_1

/** @}*/

/** @addtogroup DL_TIMERG_EVENT_ROUTE
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_EVENT_ROUTE_1
 */
#define DL_TIMERG_EVENT_ROUTE_1                           DL_TIMER_EVENT_ROUTE_1
/**
 * @brief Redirects to common @ref DL_TIMER_EVENT_ROUTE_2
 */
#define DL_TIMERG_EVENT_ROUTE_2                           DL_TIMER_EVENT_ROUTE_2

/** @}*/

/**
 * @brief Redirects to common @ref DL_Timer_enablePower
 *        使能外设电源
 */
#define DL_TimerG_enablePower                               DL_Timer_enablePower

/**
 * @brief Redirects to common @ref DL_Timer_disablePower
 *        禁用外设电源
 */
#define DL_TimerG_disablePower                             DL_Timer_disablePower

/**
 * @brief Redirects to common @ref DL_Timer_isPowerEnabled
 *        查询外设电源是否使能
 */
#define DL_TimerG_isPowerEnabled                         DL_Timer_isPowerEnabled

/**
 * @brief Redirects to common @ref DL_Timer_reset
 *        复位定时器外设
 */
#define DL_TimerG_reset                                           DL_Timer_reset

/**
 * @brief Redirects to common @ref DL_Timer_isReset
 *        查询定时器是否已复位
 */
#define DL_TimerG_isReset                                       DL_Timer_isReset

/*!
 * @brief Redirects to common @ref DL_Timer_setCCPDirection
 *        设置 CCP 引脚方向
 */
#define DL_TimerG_setCCPDirection                       DL_Timer_setCCPDirection

/*!
 * @brief Redirects to common @ref DL_Timer_getCCPDirection
 *        获取 CCP 引脚方向
 */
#define DL_TimerG_getCCPDirection                       DL_Timer_getCCPDirection

/*!
 * @brief Redirects to common @ref DL_Timer_setCCPOutputDisabled
 *        设置 CCP 输出禁用
 */
#define DL_TimerG_setCCPOutputDisabled             DL_Timer_setCCPOutputDisabled

/*!
 * @brief Redirects to common @ref DL_Timer_setClockConfig
 *        设置时钟配置
 */
#define DL_TimerG_setClockConfig                         DL_Timer_setClockConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getQEIDirection
 *        获取 QEI 方向
 */
#define DL_TimerG_getQEIDirection                       DL_Timer_getQEIDirection

/*!
 * @brief Redirects to common @ref DL_Timer_getClockConfig
 *        获取时钟配置
 */
#define DL_TimerG_getClockConfig                         DL_Timer_getClockConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getClockConfig
 *        使能定时器时钟
 */
#define DL_TimerG_enableClock                               DL_Timer_enableClock

/*!
 * @brief Redirects to common @ref DL_Timer_getClockConfig
 *        禁用定时器时钟
 */
#define DL_TimerG_disableClock                             DL_Timer_disableClock

/*!
 * @brief Redirects to common @ref DL_Timer_isClockEnabled
 *        查询定时器时钟是否使能
 */
#define DL_TimerG_isClockEnabled                         DL_Timer_isClockEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTrigger
 */
#define DL_TimerG_configCrossTrigger                 DL_Timer_configCrossTrigger


/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTriggerSrc
 */
#define DL_TimerG_configCrossTriggerSrc           DL_Timer_configCrossTriggerSrc


/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTriggerInputCond
 */
#define DL_TimerG_configCrossTriggerInputCond \
                                            DL_Timer_configCrossTriggerInputCond

/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTriggerEnable
 */
#define DL_TimerG_configCrossTriggerEnable     DL_Timer_configCrossTriggerEnable

/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerConfig
 */
#define DL_TimerG_getCrossTriggerConfig           DL_Timer_getCrossTriggerConfig


/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerSrc
 */
#define DL_TimerG_getCrossTriggerSrc                 DL_Timer_getCrossTriggerSrc


/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerInputCond
 */
#define DL_TimerG_getCrossTriggerInputCond     DL_Timer_getCrossTriggerInputCond

/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerEnable
 */
#define DL_TimerG_getCrossTriggerEnable           DL_Timer_getCrossTriggerEnable

/*!
 * @brief Redirects to common @ref DL_Timer_generateCrossTrigger
 */
#define DL_TimerG_generateCrossTrigger             DL_Timer_generateCrossTrigger

/*!
 * @brief Redirects to common @ref DL_Timer_setLoadValue
 */
#define DL_TimerG_setLoadValue                           DL_Timer_setLoadValue

/*!
 * @brief Redirects to common @ref DL_Timer_getLoadValue
 */
#define DL_TimerG_getLoadValue                           DL_Timer_getLoadValue

/*!
 * @brief Redirects to common @ref DL_Timer_getTimerCount
 */
#define DL_TimerG_getTimerCount                           DL_Timer_getTimerCount

/*!
 * @brief Redirects to common @ref DL_Timer_setTimerCount
 */
#define DL_TimerG_setTimerCount                           DL_Timer_setTimerCount

/*!
 * @brief Redirects to common @ref DL_Timer_setDebugReleaseBehavior
 */
#define DL_TimerG_setDebugReleaseBehavior       DL_Timer_setDebugReleaseBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_getDebugReleaseBehavior
 */
#define DL_TimerG_getDebugReleaseBehavior       DL_Timer_getDebugReleaseBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterControl
 */
#define DL_TimerG_setCounterControl                   DL_Timer_setCounterControl

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterZeroControl
 */
#define DL_TimerG_getCounterZeroControl           DL_Timer_getCounterZeroControl

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterAdvanceControl
 */
#define DL_TimerG_getCounterAdvanceControl     DL_Timer_getCounterAdvanceControl

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterLoadControl
 */
#define DL_TimerG_getCounterLoadControl           DL_Timer_getCounterLoadControl

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterMode
 */
#define DL_TimerG_setCounterMode                         DL_Timer_setCounterMode

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterMode
 */
#define DL_TimerG_getCounterMode                         DL_Timer_getCounterMode

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterValueAfterEnable
 */
#define DL_TimerG_setCounterValueAfterEnable DL_Timer_setCounterValueAfterEnable

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterValueAfterEnable
 */
#define DL_TimerG_getCounterValueAfterEnable DL_Timer_getCounterValueAfterEnable

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterRepeatMode
 */
#define DL_TimerG_setCounterRepeatMode             DL_Timer_setCounterRepeatMode

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterRepeatMode
 */
#define DL_TimerG_getCounterRepeatMode             DL_Timer_getCounterRepeatMode

/*!
 * @brief Redirects to common @ref DL_Timer_initTimerMode
 */
#define DL_TimerG_initTimerMode                           DL_Timer_initTimerMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCaptureMode
 */
#define DL_TimerG_initCaptureMode                       DL_Timer_initCaptureMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCaptureTriggerMode
 */
#define DL_TimerG_initCaptureTriggerMode         DL_Timer_initCaptureTriggerMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCaptureCombinedMode
 */
#define DL_TimerG_initCaptureCombinedMode       DL_Timer_initCaptureCombinedMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCompareMode
 */
#define DL_TimerG_initCompareMode                       DL_Timer_initCompareMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCompareTriggerMode
 */
#define DL_TimerG_initCompareTriggerMode         DL_Timer_initCompareTriggerMode

/*!
 * @brief Redirects to common @ref DL_Timer_initFourCCPWMMode
 */
#define DL_TimerG_initPWMMode                               DL_Timer_initFourCCPWMMode

/*!
 * @brief Redirects to common @ref DL_Timer_resetCounterMode
 */
#define DL_TimerG_resetCounterMode                     DL_Timer_resetCounterMode

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareValue
 */
#define DL_TimerG_setCaptureCompareValue         DL_Timer_setCaptureCompareValue

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareValue
 */
#define DL_TimerG_getCaptureCompareValue         DL_Timer_getCaptureCompareValue

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareCtl
 */
#define DL_TimerG_setCaptureCompareCtl             DL_Timer_setCaptureCompareCtl

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareCtl
 */
#define DL_TimerG_getCaptureCompareCtl             DL_Timer_getCaptureCompareCtl

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptCompUpdateMethod
 */
#define DL_TimerG_getCaptCompUpdateMethod       DL_Timer_getCaptCompUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptCompUpdateMethod
 */
#define DL_TimerG_setCaptCompUpdateMethod       DL_Timer_setCaptCompUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptCompUpdateMethod
 */
#define DL_TimerG_getCaptCompActUpdateMethod       DL_Timer_getCaptCompUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptCompActUpdateMethod
 */
#define DL_TimerG_setCaptCompActUpdateMethod       DL_Timer_setCaptCompActUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_enableShadowFeatures
 */
#define DL_TimerG_enableShadowFeatures             DL_Timer_enableShadowFeatures

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompSrcDn
 */
#define DL_TimerG_setSecondCompSrcDn                 DL_Timer_setSecondCompSrcDn

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompSrcDn
 */
#define DL_TimerG_getSecondCompSrcDn                 DL_Timer_GetSecondCompSrcDn

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompSrcUp
 */
#define DL_TimerG_setSecondCompSrcUp                 DL_Timer_setSecondCompSrcUp

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompSrcUp
 */
#define DL_TimerG_getSecondCompSrcUp                 DL_Timer_GetSecondCompSrcUp


/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareOutCtl
 */
#define DL_TimerG_setCaptureCompareOutCtl       DL_Timer_setCaptureCompareOutCtl

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareOutCtl
 */
#define DL_TimerG_getCaptureCompareOutCtl       DL_Timer_getCaptureCompareOutCtl

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareAction
 */
#define DL_TimerG_setCaptureCompareAction       DL_Timer_setCaptureCompareAction

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareAction
 */
#define DL_TimerG_getCaptureCompareAction       DL_Timer_getCaptureCompareAction

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompActionDn
 */
#define DL_TimerG_setSecondCompActionDn           DL_Timer_setSecondCompActionDn

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompActionDn
 */
#define DL_TimerG_getSecondCompActionDn           DL_Timer_getSecondCompActionDn

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompActionUp
 */
#define DL_TimerG_setSecondCompActionUp           DL_Timer_setSecondCompActionUp

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompActionUp
 */
#define DL_TimerG_getSecondCompActionUp           DL_Timer_getSecondCompActionUp

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareInput
 */
#define DL_TimerG_setCaptureCompareInput \
                                           DL_Timer_setCaptureCompareInput

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareInput
 */
#define DL_TimerG_getCaptureCompareInput \
                                           DL_Timer_getCaptureCompareInput

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareInputFilter
 */
#define DL_TimerG_setCaptureCompareInputFilter \
                                           DL_Timer_setCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareInputFilter
 */
#define DL_TimerG_getCaptureCompareInputFilter \
                                           DL_Timer_getCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_enableCaptureCompareInputFilter
 */
#define DL_TimerG_enableCaptureCompareInputFilter \
                                        DL_Timer_enableCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_disableCaptureCompareInputFilter
 */
#define DL_TimerG_disableCaptureCompareInputFilter \
                                       DL_Timer_disableCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_isCaptureCompareInputFilterEnabled
 */
#define DL_TimerG_isCaptureCompareInputFilterEnabled \
                                     DL_Timer_isCaptureCompareInputFilterEnabled


/*!
 * @brief Redirects to common @ref DL_Timer_setExternalTriggerEvent
 */
#define DL_TimerG_setExternalTriggerEvent       DL_Timer_setExternalTriggerEvent

/*!
 * @brief Redirects to common @ref DL_Timer_getExternalTriggerEvent
 */
#define DL_TimerG_getExternalTriggerEvent       DL_Timer_getExternalTriggerEvent

/*!
 * @brief Redirects to common @ref DL_Timer_enableExternalTrigger
 */
#define DL_TimerG_enableExternalTrigger           DL_Timer_enableExternalTrigger

/*!
 * @brief Redirects to common @ref DL_Timer_disableExternalTrigger
 */
#define DL_TimerG_disableExternalTrigger         DL_Timer_disableExternalTrigger

/*!
 * @brief Redirects to common @ref DL_Timer_isExternalTriggerEnabled
 */
#define DL_TimerG_isExternalTriggerEnabled     DL_Timer_isExternalTriggerEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_startCounter
 *        启动定时器计数
 */
#define DL_TimerG_startCounter                             DL_Timer_startCounter

/*!
 * @brief Redirects to common @ref DL_Timer_stopCounter
 *        停止定时器计数
 */
#define DL_TimerG_stopCounter                               DL_Timer_stopCounter

/*!
 * @brief Redirects to common @ref DL_Timer_isRunning
 *        查询定时器是否正在运行
 */
#define DL_TimerG_isRunning                                   DL_Timer_isRunning

/*!
 * @brief Redirects to common @ref DL_Timer_enableInterrupt
 *        使能定时器中断
 */
#define DL_TimerG_enableInterrupt                       DL_Timer_enableInterrupt

/*!
 * @brief Redirects to common @ref DL_Timer_disableInterrupt
 *        禁用定时器中断
 */
#define DL_TimerG_disableInterrupt                     DL_Timer_disableInterrupt

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledInterrupts
 *        获取已使能的中断
 */
#define DL_TimerG_getEnabledInterrupts             DL_Timer_getEnabledInterrupts

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledInterruptStatus
 *        获取已使能中断的状态
 */
#define DL_TimerG_getEnabledInterruptStatus   DL_Timer_getEnabledInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_getRawInterruptStatus
 *        获取原始中断状态
 */
#define DL_TimerG_getRawInterruptStatus           DL_Timer_getRawInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_getPendingInterrupt
 *        获取待处理中断
 */
#define DL_TimerG_getPendingInterrupt               DL_Timer_getPendingInterrupt

/*!
 * @brief Redirects to common @ref DL_Timer_clearInterruptStatus
 *        清除中断状态
 */
#define DL_TimerG_clearInterruptStatus             DL_Timer_clearInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_configQEI
 *        配置 QEI 模式
 */
#define DL_TimerG_configQEI                                   DL_Timer_configQEI

/*!
 * @brief Redirects to common @ref DL_Timer_configQEIHallInputMode
 *        配置 QEI Hall 输入模式
 */
#define DL_TimerG_configQEIHallInputMode         DL_Timer_configQEIHallInputMode

/*!
 * @brief Redirects to common @ref DL_Timer_setPublisherChanID
 */
#define DL_TimerG_setPublisherChanID                 DL_Timer_setPublisherChanID

/*!
 * @brief Redirects to common @ref DL_Timer_getPublisherChanID
 */
#define DL_TimerG_getPublisherChanID                 DL_Timer_getPublisherChanID

/*!
 * @brief Redirects to common @ref DL_Timer_setSubscriberChanID
 */
#define DL_TimerG_setSubscriberChanID               DL_Timer_setSubscriberChanID

/*!
 * @brief Redirects to common @ref DL_Timer_getSubscriberChanID
 */
#define DL_TimerG_getSubscriberChanID               DL_Timer_getSubscriberChanID

/*!
 * @brief Redirects to common @ref DL_Timer_enableEvent
 */
#define DL_TimerG_enableEvent                               DL_Timer_enableEvent

/*!
 * @brief Redirects to common @ref DL_Timer_disableEvent
 */
#define DL_TimerG_disableEvent                             DL_Timer_disableEvent

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledEvents
 */
#define DL_TimerG_getEnabledEvents                     DL_Timer_getEnabledEvents

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledEventStatus
 */
#define DL_TimerG_getEnabledEventStatus           DL_Timer_getEnabledEventStatus

/*!
 * @brief Redirects to common @ref DL_Timer_getRawEventsStatus
 */
#define DL_TimerG_getRawEventsStatus                 DL_Timer_getRawEventsStatus

/*!
 * @brief Redirects to common @ref DL_Timer_clearInterruptStatus
 */
#define DL_TimerG_clearInterruptStatus             DL_Timer_clearInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_saveConfiguration
 */
#define DL_TimerG_saveConfiguration                   DL_Timer_saveConfiguration

/*!
 * @brief Redirects to common @ref DL_Timer_restoreConfiguration
 */
#define DL_TimerG_restoreConfiguration             DL_Timer_restoreConfiguration

/*!
 * @brief Redirects to common @ref DL_Timer_setCoreHaltBehavior
 *        设置内核暂停时定时器行为
 */
#define DL_TimerG_setCoreHaltBehavior               DL_Timer_setCoreHaltBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_getCoreHaltBehavior
 *        获取内核暂停时定时器行为
 */
#define DL_TimerG_getHaltBehavior                   DL_Timer_getCoreHaltBehavior

/* clang-format on */

#ifdef __cplusplus
}
#endif

#endif /* __MSPM0_HAS_TIMER_G__ */

#endif /* ti_dl_dl_timerg__include */
/** @}*/
