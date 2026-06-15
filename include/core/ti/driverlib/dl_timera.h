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
 *  @file       dl_timera.h
 *  @brief      General Purpose Timer Advance (TIMA) Driver Library
 *              通用高级定时器 (TIMA) 驱动库
 *  @defgroup   TIMA General Purpose Timer Advance (TIMA)
 *
 *  @anchor ti_dl_dl_timera__Overview
 *  # Overview
 *
 *   The TimerA Driver Library allows you to configure the Advanced Timer
 *   (TIMA) module in output compare, input capture, PWM output, one-shot and
 *   periodic modes. For detailed TIMA features please refer to the Device
 *   Technical Reference Manual (TRM).
 *  <hr>
 ******************************************************************************
 */
/** @addtogroup TIMA
 * @{
 */
#ifndef ti_dl_dl_timera__include
#define ti_dl_dl_timera__include

#include <stdbool.h>
#include <stdint.h>

#include <ti/driverlib/dl_timer.h>

#ifdef __MSPM0_HAS_TIMER_A__

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Configuration structure to backup Timer A peripheral state before
 *        entering STOP or STANDBY mode. Used by
 *        @ref DL_TimerA_saveConfiguration and
 *        @ref DL_TimerA_restoreConfiguration
 *        进入 STOP 或 STANDBY 模式前备份 Timer A 外设状态的配置结构体
 */
typedef struct {
    /*! Timer A subscriber 0 channel ID. Value between 0 - 15.
     *  订阅者 0 通道 ID，取值范围 0-15 */
    uint32_t sub0PortConf;
    /*! Timer A subscriber 1 channel ID. Value between 0 - 15.
     *  订阅者 1 通道 ID，取值范围 0-15 */
    uint32_t sub1PortConf;
    /*! Timer A publisher 0 channel ID. Value between 0 - 15.
     *  发布者 0 通道 ID，取值范围 0-15 */
    uint32_t pub0PortConf;
    /*! Timer A publisher 1 channel ID. Value between 0 - 15.
     *  发布者 1 通道 ID，取值范围 0-15 */
    uint32_t pub1PortConf;
    /*! Timer A clockDivider configuration.
     *  时钟分频器配置 */
    uint32_t clkDivConf;
    /*! Timer A clock prescaler configuration.
     *  时钟预分频器配置 */
    uint32_t clockPscConf;
    /*! Timer A Clock Source Select configuration.
     *  时钟源选择配置 */
    uint32_t clkSelConf;
    /*! Timer A Clock configuration.
     *  计数时钟配置 */
    uint32_t countClkConf;
    /*! Timer A interrupt configuration for EVENT0.
     *  事件 0 中断配置 */
    uint32_t intEvnt0Conf;
    /*! Timer A interrupt configuration for EVENT1.
     *  事件 1 中断配置 */
    uint32_t intEvnt1Conf;
    /*! Timer A interrupt configuration for EVENT2.
     *  事件 2 中断配置 */
    uint32_t intEvnt2Conf;
    /*! Timer A CCP Direction configuration.
     *  CCP 引脚方向配置 */
    uint32_t ccpDirConf;
    /*! Timer A CCP Output forced low configuration.
     *  CCP 输出强制低电平配置 */
    uint32_t outDisConf;
    /*! Timer A Cross Timer Trigger Control configuration.
     *  跨定时器触发控制配置 */
    uint32_t crossTrigCtl;
    /*! Timer A Cross Timer Trigger Mask
     *  跨定时器触发掩码 */
    uint32_t crossTrifMsk;
    /*! Timer A Cross Trigger Configuration
     *  交叉触发选择配置 */
    uint32_t tSelConf;
    /*! Timer A Cross Timer Trigger
     *  跨定时器触发配置 */
    uint32_t crossTrigConf;
    /*! Timer A counter value. Timer counter value is stored but user can
     *  specify if counter is restore via restoreCounter argument in
     *  @ref DL_TimerA_restoreConfiguration
     *  计数器值，可通过 restoreCounter 参数决定是否恢复 */
    uint32_t cntVal;
    /*! Timer A counter control configuration
     *  计数器控制配置 */
    uint32_t cntCtlConf;
    /*! Timer A load value
     *  装载值 */
    uint32_t loadVal;
    /*! Timer A Capture or Compare 0 value
     *  捕获/比较通道 0 值 */
    uint32_t cc0Val;
    /*! Timer A Capture or Compare 1 value
     *  捕获/比较通道 1 值 */
    uint32_t cc1Val;
    /*! Timer A Capture or Compare 2 value
     *  捕获/比较通道 2 值 */
    uint32_t cc2Val;
    /*! Timer A Capture or Compare 3 value
     *  捕获/比较通道 3 值 */
    uint32_t cc3Val;
    /*! Timer A Capture or Compare Control Register 0
     *  捕获/比较控制寄存器 0 */
    uint32_t cc0Ctl;
    /*! Timer A Capture or Compare Control Register 1
     *  捕获/比较控制寄存器 1 */
    uint32_t cc1Ctl;
    /*! Timer A Capture or Compare Control Register 2
     *  捕获/比较控制寄存器 2 */
    uint32_t cc2Ctl;
    /*! Timer A Capture or Compare Control Register 3
     *  捕获/比较控制寄存器 3 */
    uint32_t cc3Ctl;
    /*! Timer A Capture or Compare Output Control Register 0
     *  捕获/比较输出控制寄存器 0 */
    uint32_t cc0OutCtl;
    /*! Timer A Capture or Compare Output Control Register 1
     *  捕获/比较输出控制寄存器 1 */
    uint32_t cc1OutCtl;
    /*! Timer A Capture or Compare Output Control Register 2
     *  捕获/比较输出控制寄存器 2 */
    uint32_t cc2OutCtl;
    /*! Timer A Capture or Compare Output Control Register 3
     *  捕获/比较输出控制寄存器 3 */
    uint32_t cc3OutCtl;
    /*! Timer A Capture or Compare Signal Generator Action Control Register 0
     *  捕获/比较信号发生器动作控制寄存器 0 */
    uint32_t cc0ActCtl;
    /*! Timer A Capture or Compare Signal Generator Action Control Register 1
     *  捕获/比较信号发生器动作控制寄存器 1 */
    uint32_t cc1ActCtl;
    /*! Timer A Capture or Compare Signal Generator Action Control Register 2
     *  捕获/比较信号发生器动作控制寄存器 2 */
    uint32_t cc2ActCtl;
    /*! Timer A Capture or Compare Signal Generator Action Control Register 3
     *  捕获/比较信号发生器动作控制寄存器 3 */
    uint32_t cc3ActCtl;
    /*! Timer A Capture or Compare Input Filter and Inversion Control
     *  Register 0
     *  捕获/比较输入滤波和反相控制寄存器 0 */
    uint32_t in0FiltCtl;
    /*! Timer A Capture or Compare Input Filter and Inversion Control
     *  Register 1
     *  捕获/比较输入滤波和反相控制寄存器 1 */
    uint32_t in1FiltCtl;
    /*! Timer A Capture or Compare Input Filter and Inversion Control
     *  Register 2
     *  捕获/比较输入滤波和反相控制寄存器 2 */
    uint32_t in2FiltCtl;
    /*! Timer A Capture or Compare Input Filter and Inversion Control
     *  Register 3
     *  捕获/比较输入滤波和反相控制寄存器 3 */
    uint32_t in3FiltCtl;
    /*! Timer A Phase Load Value
     *  相位装载值 */
    uint32_t phaseVal;
    /*! Timer A Dead Band Control Configuration
     *  死区控制配置 */
    uint32_t dbCtlConf;
    /*! Timer A Repeat Counter Configuration
     *  重复计数器配置 */
    uint32_t rcConf;
    /*! Timer A Fault Source and Handling Configuration
     *  故障源和处理配置 */
    uint32_t faultSrcHndlConf;
    /*! Timer A Fault Source Input Filtering Configuration
     *  故障源输入滤波配置 */
    uint32_t faultInCtl;
    /*! Timer A Fault Cross Trigger Input Selection Configuration
     *  故障交叉触发输入选择配置 */
    uint32_t faultCrossTrigCtl;
    /*! Boolean flag indicating whether or not a valid configuration structure
     *  exists. Should not be modified by the user.
     *  备份就绪标志，用户不应修改此字段 */
    bool backupRdy;
} DL_TimerA_backupConfig;

/* clang-format off */

/** @addtogroup DL_TIMERA_CAPTURE_COMPARE_INDEX
 *  @{
 */

/**
 * @brief  Index associated to Capture Compare 0
 *         捕获比较通道 0 索引
 */
#define DL_TIMERA_CAPTURE_COMPARE_0_INDEX       DL_TIMER_CC_0_INDEX

/**
 * @brief  Index associated to Capture Compare 1
 *         捕获比较通道 1 索引
 */
#define DL_TIMERA_CAPTURE_COMPARE_1_INDEX       DL_TIMER_CC_1_INDEX

/**
 * @brief  Index associated to Capture Compare 2
 *         捕获比较通道 2 索引
 * @note   <b>This option is not supported by all device TimerA modules </b>
 *         please refer to the "TIMx Configurations" table in the
 *         device specific datasheet to determine Timer instances which
 *         support "4 CC Channel" configuration.
 */
#define DL_TIMERA_CAPTURE_COMPARE_2_INDEX       DL_TIMER_CC_2_INDEX

/**
 * @brief  Index associated to Capture Compare 3
 *         捕获比较通道 3 索引
 * @note   <b>This option is not supported by all device TimerA modules </b>
 *         please refer to the "TIMx Configurations" table in the
 *         device specific datasheet to determine Timer instances which
 *         support "4 CC Channel" configuration.
 */
#define DL_TIMERA_CAPTURE_COMPARE_3_INDEX       DL_TIMER_CC_3_INDEX

/**
 * @brief  Index associated to Capture Compare 4
 *         捕获比较通道 4 索引
 * @note   <b>This option is not supported by all device TimerA modules </b>
 *         please refer to the "TIMx Configurations" table in the
 *         device specific datasheet to determine Timer instances which
 *         support "6 CC Channel" configuration with 4 external and 2 internal
 *         CC channels.
 */
#define DL_TIMERA_CAPTURE_COMPARE_4_INDEX       DL_TIMER_CC_4_INDEX

/**
 * @brief  Index associated to Capture Compare 5
 *         捕获比较通道 5 索引
 * @note   <b>This option is not supported by all device TimerA modules </b>
 *         please refer to the "TIMx Configurations" table in the
 *         device specific datasheet to determine Timer instances which
 *         support "6 CC Channel" configuration with 4 external and 2 internal
 *         CC channels.
 */
#define DL_TIMERA_CAPTURE_COMPARE_5_INDEX       DL_TIMER_CC_5_INDEX

/** @}*/

/** @addtogroup DL_TIMER_CCx
 *  @{
 */
/*!
 * @brief Selects Output direction for CCP0
 *        选择 CCP2 为输出方向
 */
#define DL_TIMERA_CC2_OUTPUT                     (GPTIMER_CCPD_C0CCP2_OUTPUT)

/*!
 * @brief Selects Input direction for CCP0
 *        选择 CCP2 为输入方向
 */
#define DL_TIMERA_CC2_INPUT                      (GPTIMER_CCPD_C0CCP2_INPUT)

/*!
 * @brief Selects Output direction for CCP1
 *        选择 CCP3 为输出方向
 */
#define DL_TIMERA_CC3_OUTPUT                     (GPTIMER_CCPD_C0CCP3_OUTPUT)

/*!
 * @brief Selects Input direction for CCP1
 *        选择 CCP3 为输入方向
 */
#define DL_TIMERA_CC3_INPUT                      (GPTIMER_CCPD_C0CCP3_INPUT)

/** @}*/

/** @addtogroup DL_TIMERA_INTERRUPT
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_INTERRUPT_ZERO_EVENT
 *        计数器归零中断事件
 */
#define DL_TIMERA_INTERRUPT_ZERO_EVENT           (DL_TIMER_INTERRUPT_ZERO_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_LOAD_EVENT
*        计数器装载中断事件
*/
#define DL_TIMERA_INTERRUPT_LOAD_EVENT           (DL_TIMER_INTERRUPT_LOAD_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC0_DN_EVENT
*        CC0 向下计数中断事件
*/
#define DL_TIMERA_INTERRUPT_CC0_DN_EVENT       (DL_TIMER_INTERRUPT_CC0_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC1_DN_EVENT
*        CC1 向下计数中断事件
*/
#define DL_TIMERA_INTERRUPT_CC1_DN_EVENT       (DL_TIMER_INTERRUPT_CC1_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC0_UP_EVENT
*        CC0 向上计数中断事件
*/
#define DL_TIMERA_INTERRUPT_CC0_UP_EVENT       (DL_TIMER_INTERRUPT_CC0_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC1_UP_EVENT
*        CC1 向上计数中断事件
*/
#define DL_TIMERA_INTERRUPT_CC1_UP_EVENT       (DL_TIMER_INTERRUPT_CC1_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_OVERFLOW_EVENT
*        计数器溢出中断事件
*/
#define DL_TIMERA_INTERRUPT_OVERFLOW_EVENT   (DL_TIMER_INTERRUPT_OVERFLOW_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC2_DN_EVENT
*/
#define DL_TIMERA_INTERRUPT_CC2_DN_EVENT       (DL_TIMER_INTERRUPT_CC2_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC3_DN_EVENT
*/
#define DL_TIMERA_INTERRUPT_CC3_DN_EVENT       (DL_TIMER_INTERRUPT_CC3_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC2_UP_EVENT
*/
#define DL_TIMERA_INTERRUPT_CC2_UP_EVENT      (DL_TIMER_INTERRUPT_CC2_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC3_UP_EVENT
*/
#define DL_TIMERA_INTERRUPT_CC3_UP_EVENT      (DL_TIMER_INTERRUPT_CC3_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC4_DN_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_INTERRUPT_CC4_DN_EVENT       (DL_TIMER_INTERRUPT_CC4_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC5_DN_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_INTERRUPT_CC5_DN_EVENT       (DL_TIMER_INTERRUPT_CC5_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC4_UP_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_INTERRUPT_CC4_UP_EVENT       (DL_TIMER_INTERRUPT_CC4_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_CC5_UP_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_INTERRUPT_CC5_UP_EVENT       (DL_TIMER_INTERRUPT_CC5_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_REPC_EVENT
*        重复计数器中断事件
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_INTERRUPT_REPC_EVENT           (DL_TIMER_INTERRUPT_REPC_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_INTERRUPT_FAULT_EVENT
*        故障中断事件
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_INTERRUPT_FAULT_EVENT         (DL_TIMER_INTERRUPT_FAULT_EVENT)

/** @}*/

/** @addtogroup DL_TIMERA_EVENT
 *  @{
 */
/*!
 * @brief Redirects to common @ref DL_TIMER_EVENT_ZERO_EVENT
 */
#define DL_TIMERA_EVENT_ZERO_EVENT                   (DL_TIMER_EVENT_ZERO_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_LOAD_EVENT
*/
#define DL_TIMERA_EVENT_LOAD_EVENT                   (DL_TIMER_EVENT_LOAD_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC0_DN_EVENT
*/
#define DL_TIMERA_EVENT_CC0_DN_EVENT               (DL_TIMER_EVENT_CC0_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC1_DN_EVENT
*/
#define DL_TIMERA_EVENT_CC1_DN_EVENT               (DL_TIMER_EVENT_CC1_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC0_UP_EVENT
*/
#define DL_TIMERA_EVENT_CC0_UP_EVENT               (DL_TIMER_EVENT_CC0_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC1_UP_EVENT
*/
#define DL_TIMERA_EVENT_CC1_UP_EVENT               (DL_TIMER_EVENT_CC1_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_OVERFLOW_EVENT
*/
#define DL_TIMERA_EVENT_OVERFLOW_EVENT           (DL_TIMER_EVENT_OVERFLOW_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC2_DN_EVENT
*/
#define DL_TIMERA_EVENT_CC2_DN_EVENT               (DL_TIMER_EVENT_CC2_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC3_DN_EVENT
*/
#define DL_TIMERA_EVENT_CC3_DN_EVENT               (DL_TIMER_EVENT_CC3_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC2_UP_EVENT
*/
#define DL_TIMERA_EVENT_CC2_UP_EVENT               (DL_TIMER_EVENT_CC2_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC3_UP_EVENT
*/
#define DL_TIMERA_EVENT_CC3_UP_EVENT               (DL_TIMER_EVENT_CC3_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC4_DN_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_EVENT_CC4_DN_EVENT               (DL_TIMER_EVENT_CC4_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC5_DN_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_EVENT_CC5_DN_EVENT               (DL_TIMER_EVENT_CC5_DN_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC4_UP_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_EVENT_CC4_UP_EVENT               (DL_TIMER_EVENT_CC4_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_CC5_UP_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_EVENT_CC5_UP_EVENT               (DL_TIMER_EVENT_CC5_UP_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_REPC_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_EVENT_REPC_EVENT                   (DL_TIMER_EVENT_REPC_EVENT)

/*!
* @brief Redirects to common @ref DL_TIMER_EVENT_FAULT_EVENT
* @note <b> This is a Timer A specific interrupt. </b>
*/
#define DL_TIMERA_EVENT_FAULT_EVENT                 (DL_TIMER_EVENT_FAULT_EVENT)

/** @}*/

/** @addtogroup DL_TIMERA_CCP0_DIS_OUT_ADV
 *  @{
 */

/**
  * @brief Redirects to common @ref DL_TIMER_CCP0_DIS_OUT_ADV_FORCE_LOW
  */
#define DL_TIMERA_CCP0_DIS_OUT_ADV_FORCE_LOW \
                                             DL_TIMER_CCP0_DIS_OUT_ADV_FORCE_LOW

/**
  * @brief Redirects to common @ref DL_TIMER_CCP0_DIS_OUT_ADV_SET_BY_OCTL
  */
#define DL_TIMERA_CCP0_DIS_OUT_ADV_SET_BY_OCTL \
                                           DL_TIMER_CCP0_DIS_OUT_ADV_SET_BY_OCTL

/** @}*/

/** @addtogroup DL_TIMERA_CCP1_DIS_OUT_ADV
 *  @{
 */

/**
  * @brief Redirects to common @ref DL_TIMER_CCP1_DIS_OUT_ADV_FORCE_LOW
  */
#define DL_TIMERA_CCP1_DIS_OUT_ADV_FORCE_LOW \
                                             DL_TIMER_CCP1_DIS_OUT_ADV_FORCE_LOW

/**
  * @brief Redirects to common @ref DL_TIMER_CCP1_DIS_OUT_ADV_SET_BY_OCTL
  */
#define DL_TIMERA_CCP1_DIS_OUT_ADV_SET_BY_OCTL \
                                           DL_TIMER_CCP1_DIS_OUT_ADV_SET_BY_OCTL
/** @}*/

/** @addtogroup DL_TIMERA_CCP2_DIS_OUT_ADV
 *  @{
 */
/**
  * @brief Redirects to common @ref DL_TIMER_CCP2_DIS_OUT_ADV_FORCE_LOW
  */
#define DL_TIMERA_CCP2_DIS_OUT_ADV_FORCE_LOW \
                                             DL_TIMER_CCP2_DIS_OUT_ADV_FORCE_LOW

/**
  * @brief Redirects to common @ref DL_TIMER_CCP2_DIS_OUT_ADV_SET_BY_OCTL
  */
#define DL_TIMERA_CCP2_DIS_OUT_ADV_SET_BY_OCTL \
                                           DL_TIMER_CCP2_DIS_OUT_ADV_SET_BY_OCTL
/** @}*/

/** @addtogroup DL_TIMERA_CCP3_DIS_OUT_ADV
 *  @{
 */

/**
  * @brief Redirects to common @ref DL_TIMER_CCP3_DIS_OUT_ADV_FORCE_LOW
  */
#define DL_TIMERA_CCP3_DIS_OUT_ADV_FORCE_LOW \
                                             DL_TIMER_CCP3_DIS_OUT_ADV_FORCE_LOW

/**
  * @brief Redirects to common @ref DL_TIMER_CCP3_DIS_OUT_ADV_SET_BY_OCTL
  */
#define DL_TIMERA_CCP3_DIS_OUT_ADV_SET_BY_OCTL \
                                           DL_TIMER_CCP3_DIS_OUT_ADV_SET_BY_OCTL
/** @}*/

/** @addtogroup DL_TIMERA_IIDX
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_ZERO
 */
#define DL_TIMERA_IIDX_ZERO                                   DL_TIMER_IIDX_ZERO

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_LOAD
 */
#define DL_TIMERA_IIDX_LOAD                                   DL_TIMER_IIDX_LOAD

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC0_DN
 */
#define DL_TIMERA_IIDX_CC0_DN                               DL_TIMER_IIDX_CC0_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC1_DN
 */
#define DL_TIMERA_IIDX_CC1_DN                               DL_TIMER_IIDX_CC1_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC2_DN
 */
#define DL_TIMERA_IIDX_CC2_DN                               DL_TIMER_IIDX_CC2_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC3_DN
 */
#define DL_TIMERA_IIDX_CC3_DN                               DL_TIMER_IIDX_CC3_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC0_UP
 */
#define DL_TIMERA_IIDX_CC0_UP                               DL_TIMER_IIDX_CC0_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC1_UP
 */
#define DL_TIMERA_IIDX_CC1_UP                               DL_TIMER_IIDX_CC1_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC2_UP
 */
#define DL_TIMERA_IIDX_CC2_UP                               DL_TIMER_IIDX_CC2_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC3_UP
 */
#define DL_TIMERA_IIDX_CC3_UP                               DL_TIMER_IIDX_CC3_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC4_DN
 */
#define DL_TIMERA_IIDX_CC4_DN                               DL_TIMER_IIDX_CC4_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC5_DN
 */
#define DL_TIMERA_IIDX_CC5_DN                               DL_TIMER_IIDX_CC5_DN

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC4_UP
 */
#define DL_TIMERA_IIDX_CC4_UP                               DL_TIMER_IIDX_CC4_UP

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_CC5_UP
 */
#define DL_TIMERA_IIDX_CC5_UP                               DL_TIMER_IIDX_CC5_UP


/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_FAULT
 */
#define DL_TIMERA_IIDX_FAULT                                 DL_TIMER_IIDX_FAULT

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_OVERFLOW
 */
#define DL_TIMERA_IIDX_OVERFLOW                           DL_TIMER_IIDX_OVERFLOW

/**
 * @brief Redirects to common @ref DL_TIMER_IIDX_REPEAT_COUNT
 */
#define DL_TIMERA_IIDX_REPEAT_COUNT                   DL_TIMER_IIDX_REPEAT_COUNT

/** @}*/

/** @addtogroup DL_TIMERA_FAULT_SOURCE
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP0_DISABLE
 */
#define DL_TIMERA_FAULT_SOURCE_COMP0_DISABLE \
                                             DL_TIMER_FAULT_SOURCE_COMP0_DISABLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP0_SENSE_LOW
 */
#define DL_TIMERA_FAULT_SOURCE_COMP0_SENSE_LOW \
                                           DL_TIMER_FAULT_SOURCE_COMP0_SENSE_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP0_SENSE_HIGH
 */
#define DL_TIMERA_FAULT_SOURCE_COMP0_SENSE_HIGH \
                                          DL_TIMER_FAULT_SOURCE_COMP0_SENSE_HIGH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP1_DISABLE
*/
#define DL_TIMERA_FAULT_SOURCE_COMP1_DISABLE \
                                             DL_TIMER_FAULT_SOURCE_COMP1_DISABLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP1_SENSE_LOW
 */
#define DL_TIMERA_FAULT_SOURCE_COMP1_SENSE_LOW \
                                           DL_TIMER_FAULT_SOURCE_COMP1_SENSE_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP1_SENSE_HIGH
 */
#define DL_TIMERA_FAULT_SOURCE_COMP1_SENSE_HIGH \
                                          DL_TIMER_FAULT_SOURCE_COMP1_SENSE_HIGH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP2_DISABLE
 */
#define DL_TIMERA_FAULT_SOURCE_COMP2_DISABLE \
                                             DL_TIMER_FAULT_SOURCE_COMP2_DISABLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP2_SENSE_LOW
 */
#define DL_TIMERA_FAULT_SOURCE_COMP2_SENSE_LOW \
                                           DL_TIMER_FAULT_SOURCE_COMP2_SENSE_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_COMP2_SENSE_HIGH
 */
#define DL_TIMERA_FAULT_SOURCE_COMP2_SENSE_HIGH \
                                          DL_TIMER_FAULT_SOURCE_COMP2_SENSE_HIGH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_0_DISABLE
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_0_DISABLE \
                                        DL_TIMER_FAULT_SOURCE_EXTERNAL_0_DISABLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_0_SENSE_LOW
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_0_SENSE_LOW \
                                      DL_TIMER_FAULT_SOURCE_EXTERNAL_0_SENSE_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_0_SENSE_HIGH
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_0_SENSE_HIGH \
                                     DL_TIMER_FAULT_SOURCE_EXTERNAL_0_SENSE_HIGH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_1_DISABLE
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_1_DISABLE \
                                        DL_TIMER_FAULT_SOURCE_EXTERNAL_1_DISABLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_1_SENSE_LOW
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_1_SENSE_LOW \
                                      DL_TIMER_FAULT_SOURCE_EXTERNAL_1_SENSE_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_1_SENSE_HIGH
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_1_SENSE_HIGH \
                                     DL_TIMER_FAULT_SOURCE_EXTERNAL_1_SENSE_HIGH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_2_DISABLE
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_2_DISABLE \
                                        DL_TIMER_FAULT_SOURCE_EXTERNAL_2_DISABLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_2_SENSE_LOW
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_2_SENSE_LOW \
                                      DL_TIMER_FAULT_SOURCE_EXTERNAL_2_SENSE_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_SOURCE_EXTERNAL_2_SENSE_HIGH
 */
#define DL_TIMERA_FAULT_SOURCE_EXTERNAL_2_SENSE_HIGH \
                                     DL_TIMER_FAULT_SOURCE_EXTERNAL_2_SENSE_HIGH

/** @}*/

/** @addtogroup DL_TIMER_FAULT_ENTRY_CCP
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CCP_DISABLED
 */
#define DL_TIMERA_FAULT_ENTRY_CCP_DISABLED     DL_TIMER_FAULT_ENTRY_CCP_DISABLED

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CCP_HIGH
 */
#define DL_TIMERA_FAULT_ENTRY_CCP_HIGH             DL_TIMER_FAULT_ENTRY_CCP_HIGH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CCP_LOW
 */
#define DL_TIMERA_FAULT_ENTRY_CCP_LOW               DL_TIMER_FAULT_ENTRY_CCP_LOW

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CCP_TOGGLE
 */
#define DL_TIMERA_FAULT_ENTRY_CCP_TOGGLE         DL_TIMER_FAULT_ENTRY_CCP_TOGGLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CCP_HIGHZ
 */
#define DL_TIMERA_FAULT_ENTRY_CCP_HIGHZ           DL_TIMER_FAULT_ENTRY_CCP_HIGHZ


/** @}*/

/** @addtogroup DL_TIMER_FAULT_EXIT_CCP
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CCP_DISABLED
 */
#define DL_TIMERA_FAULT_EXIT_CCP_DISABLED       DL_TIMER_FAULT_EXIT_CCP_DISABLED
/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CCP_HIGH
 */
#define DL_TIMERA_FAULT_EXIT_CCP_HIGH               DL_TIMER_FAULT_EXIT_CCP_HIGH
/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CCP_LOW
 */
#define DL_TIMERA_FAULT_EXIT_CCP_LOW                 DL_TIMER_FAULT_EXIT_CCP_LOW
/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CCP_TOGGLE
 */
#define DL_TIMERA_FAULT_EXIT_CCP_TOGGLE           DL_TIMER_FAULT_EXIT_CCP_TOGGLE

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CCP_HIGHZ
 */
#define DL_TIMERA_FAULT_EXIT_CCP_HIGHZ             DL_TIMER_FAULT_EXIT_CCP_HIGHZ

/** @}*/

/** @addtogroup DL_TIMER_FAULT_EXIT_CTR
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CTR_RESUME
 */
#define DL_TIMERA_FAULT_EXIT_CTR_RESUME           DL_TIMER_FAULT_EXIT_CTR_RESUME

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_EXIT_CTR_CVAE_ACTION
 */
#define DL_TIMERA_FAULT_EXIT_CTR_CVAE_ACTION DL_TIMER_FAULT_EXIT_CTR_CVAE_ACTION

/** @}*/

/** @addtogroup DL_TIMER_FAULT_ENTRY_CTR
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CTR_CONT_COUNT
 */
#define DL_TIMERA_FAULT_ENTRY_CTR_CONT_COUNT DL_TIMER_FAULT_ENTRY_CTR_CONT_COUNT

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_ENTRY_CTR_SUSP_COUNT
 */
#define DL_TIMERA_FAULT_ENTRY_CTR_SUSP_COUNT DL_TIMER_FAULT_ENTRY_CTR_SUSP_COUNT

/** @}*/

/** @addtogroup DL_TIMERA_FAULT_CONFIG_TFIM
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_TFIM_DISABLED
 */
#define DL_TIMERA_FAULT_CONFIG_TFIM_DISABLED \
                                             DL_TIMER_FAULT_CONFIG_TFIM_DISABLED

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_TFIM_ENABLED
 */
#define DL_TIMERA_FAULT_CONFIG_TFIM_ENABLED   DL_TIMER_FAULT_CONFIG_TFIM_ENABLED

/** @}*/


/** @addtogroup DL_TIMERA_FAULT_CONFIG_FL
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FL_NO_LATCH
 */
#define DL_TIMERA_FAULT_CONFIG_FL_NO_LATCH     DL_TIMER_FAULT_CONFIG_FL_NO_LATCH

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FL_NO_LATCH
 */
#define DL_TIMERA_FAULT_CONFIG_FL_LATCH_SW_CLR \
                                           DL_TIMER_FAULT_CONFIG_FL_LATCH_SW_CLR

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FL_LATCH_Z_CLR
 */
#define DL_TIMERA_FAULT_CONFIG_FL_LATCH_Z_CLR \
                                            DL_TIMER_FAULT_CONFIG_FL_LATCH_Z_CLR

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FL_LATCH_LD_CLR
 */
#define DL_TIMERA_FAULT_CONFIG_FL_LATCH_LD_CLR \
                                           DL_TIMER_FAULT_CONFIG_FL_LATCH_LD_CLR

/** @}*/


/** @addtogroup DL_TIMERA_FAULT_CONFIG_FI
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FI_INDEPENDENT
 */
#define DL_TIMERA_FAULT_CONFIG_FI_INDEPENDENT \
                                            DL_TIMER_FAULT_CONFIG_FI_INDEPENDENT

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FI_DEPENDENT
 */
#define DL_TIMERA_FAULT_CONFIG_FI_DEPENDENT   DL_TIMER_FAULT_CONFIG_FI_DEPENDENT

/** @}*/

/** @addtogroup DL_TIMERA_FAULT_CONFIG_FIEN
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FIEN_DISABLED
 */
#define DL_TIMERA_FAULT_CONFIG_FIEN_DISABLED DL_TIMER_FAULT_CONFIG_FIEN_DISABLED

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_CONFIG_FIEN_ENABLED
 */
#define DL_TIMERA_FAULT_CONFIG_FIEN_ENABLED   DL_TIMER_FAULT_CONFIG_FIEN_ENABLED

/** @}*/

/** @addtogroup DL_TIMERA_FAULT_FILT
 *  @{
 */
/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_BYPASS
 */
#define DL_TIMERA_FAULT_FILTER_BYPASS               DL_TIMER_FAULT_FILTER_BYPASS

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_FILTERED
 */
#define DL_TIMERA_FAULT_FILTER_FILTERED           DL_TIMER_FAULT_FILTER_FILTERED

/** @}*/

/** @addtogroup DL_TIMERA_FAULT_FILTER_CPV
 *  @{
 */

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_CPV_CONSEC_PER
 */
#define DL_TIMERA_FAULT_FILTER_CPV_CONSEC_PER \
                                            DL_TIMER_FAULT_FILTER_CPV_CONSEC_PER

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_CPV_VOTING
 */
#define DL_TIMERA_FAULT_FILTER_CPV_VOTING       DL_TIMER_FAULT_FILTER_CPV_VOTING


/** @}*/

/** @addtogroup DL_TIMERA_FAULT_FILTER_FP
 *  @{
 */
/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_FP_PER_3
 */
#define DL_TIMERA_FAULT_FILTER_FP_PER_3           DL_TIMER_FAULT_FILTER_FP_PER_3

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_FP_PER_5
 */
#define DL_TIMERA_FAULT_FILTER_FP_PER_5           DL_TIMER_FAULT_FILTER_FP_PER_5

/*!
 * @brief Redirects to common @ref DL_TIMER_FAULT_FILTER_FP_PER_8
 */
#define DL_TIMERA_FAULT_FILTER_FP_PER_8           DL_TIMER_FAULT_FILTER_FP_PER_8

/** @}*/

/** @addtogroup DL_TIMERA_PUBLISHER_INDEX
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_PUBLISHER_INDEX_0
 */
#define DL_TIMERA_PUBLISHER_INDEX_0                   DL_TIMER_PUBLISHER_INDEX_0

/**
 * @brief Redirects to common @ref DL_TIMER_PUBLISHER_INDEX_1
 */
#define DL_TIMERA_PUBLISHER_INDEX_1                   DL_TIMER_PUBLISHER_INDEX_1

/** @}*/

/** @addtogroup DL_TIMERA_SUBSCRIBER_INDEX
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_SUBSCRIBER_INDEX_0
 */
#define DL_TIMERA_SUBSCRIBER_INDEX_0                 DL_TIMER_SUBSCRIBER_INDEX_0
/**
 * @brief Redirects to common @ref DL_TIMER_SUBSCRIBER_INDEX_1
 */
#define DL_TIMERA_SUBSCRIBER_INDEX_1                 DL_TIMER_SUBSCRIBER_INDEX_1

/** @}*/

/** @addtogroup DL_TIMERA_EVENT_ROUTE
 *  @{
 */

/**
 * @brief Redirects to common @ref DL_TIMER_EVENT_ROUTE_1
 */
#define DL_TIMERA_EVENT_ROUTE_1                           DL_TIMER_EVENT_ROUTE_1
/**
 * @brief Redirects to common @ref DL_TIMER_EVENT_ROUTE_2
 */
#define DL_TIMERA_EVENT_ROUTE_2                           DL_TIMER_EVENT_ROUTE_2

/** @}*/

/**
 * @brief Redirects to common @ref DL_Timer_PWMConfig
 *        PWM 配置结构体别名
 */
typedef DL_Timer_PWMConfig                              DL_TimerA_PWMConfig;

/**
 * @brief Redirects to common @ref DL_Timer_ClockConfig
 *        时钟配置结构体别名
 */
typedef DL_Timer_ClockConfig                              DL_TimerA_ClockConfig;

/**
 * @brief Redirects to common @ref DL_Timer_TimerConfig
 *        定时器配置结构体别名
 */
typedef DL_Timer_TimerConfig                              DL_TimerA_TimerConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CaptureConfig
 *        捕获配置结构体别名
 */
typedef DL_Timer_CaptureConfig                         DL_TimerA_CaptureConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CaptureTriggerConfig
 *        捕获触发配置结构体别名
 */
typedef DL_Timer_CaptureTriggerConfig            DL_TimerA_CaptureTriggerConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CaptureCombinedConfig
 *        组合捕获配置结构体别名
 */
typedef DL_Timer_CaptureCombinedConfig          DL_TimerA_CaptureCombinedConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CompareConfig
 *        比较配置结构体别名
 */
typedef DL_Timer_CompareConfig                          DL_TimerA_CompareConfig;

/**
 * @brief Redirects to common @ref DL_Timer_CompareTriggerConfig
 *        比较触发配置结构体别名
 */
typedef DL_Timer_CompareTriggerConfig            DL_TimerA_CompareTriggerConfig;
/**
 * @brief Redirects to common @ref DL_Timer_enablePower
 *        使能外设电源
 */
#define DL_TimerA_enablePower                               DL_Timer_enablePower

/**
 * @brief Redirects to common @ref DL_Timer_disablePower
 *        禁用外设电源
 */
#define DL_TimerA_disablePower                             DL_Timer_disablePower

/**
 * @brief Redirects to common @ref DL_Timer_isPowerEnabled
 *        查询外设电源是否使能
 */
#define DL_TimerA_isPowerEnabled                         DL_Timer_isPowerEnabled

/**
 * @brief Redirects to common @ref DL_Timer_reset
 *        复位定时器外设
 */
#define DL_TimerA_reset                                           DL_Timer_reset

/**
 * @brief Redirects to common @ref DL_Timer_isReset
 *        查询定时器是否已复位
 */
#define DL_TimerA_isReset                                       DL_Timer_isReset

/*!
 * @brief Redirects to common @ref DL_Timer_setCCPDirection
 *        设置 CCP 引脚方向
 */
#define DL_TimerA_setCCPDirection                       DL_Timer_setCCPDirection

/*!
 * @brief Redirects to common @ref DL_Timer_getCCPDirection
 *        获取 CCP 引脚方向
 */
#define DL_TimerA_getCCPDirection                       DL_Timer_getCCPDirection

/*!
 * @brief Redirects to common @ref DL_Timer_setCCPOutputDisabled
 *        设置 CCP 输出禁用
 */
#define DL_TimerA_setCCPOutputDisabled             DL_Timer_setCCPOutputDisabled

/*!
 * @brief Redirects to common @ref DL_Timer_setCCPOutputDisabledAdv
 *        设置 CCP 输出禁用（高级模式）
 */
#define DL_TimerA_setCCPOutputDisabledAdv       DL_Timer_setCCPOutputDisabledAdv

/*!
 * @brief Redirects to common @ref DL_Timer_setClockConfig
 *        设置时钟配置
 */
#define DL_TimerA_setClockConfig                         DL_Timer_setClockConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getClockConfig
 *        获取时钟配置
 */
#define DL_TimerA_getClockConfig                         DL_Timer_getClockConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getClockConfig
 *        使能定时器时钟
 */
#define DL_TimerA_enableClock                               DL_Timer_enableClock

/*!
 * @brief Redirects to common @ref DL_Timer_getClockConfig
 *        禁用定时器时钟
 */
#define DL_TimerA_disableClock                             DL_Timer_disableClock

/*!
 * @brief Redirects to common @ref DL_Timer_isClockEnabled
 *        查询定时器时钟是否使能
 */
#define DL_TimerA_isClockEnabled                         DL_Timer_isClockEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTrigger
 *        配置跨定时器触发
 */
#define DL_TimerA_configCrossTrigger                 DL_Timer_configCrossTrigger


/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTriggerSrc
 *        配置跨定时器触发源
 */
#define DL_TimerA_configCrossTriggerSrc           DL_Timer_configCrossTriggerSrc


/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTriggerInputCond
 *        配置跨定时器触发输入条件
 */
#define DL_TimerA_configCrossTriggerInputCond \
                                            DL_Timer_configCrossTriggerInputCond

/*!
 * @brief Redirects to common @ref DL_Timer_configCrossTriggerEnable
 *        使能跨定时器触发
 */
#define DL_TimerA_configCrossTriggerEnable     DL_Timer_configCrossTriggerEnable

/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerConfig
 *        获取跨定时器触发配置
 */
#define DL_TimerA_getCrossTriggerConfig           DL_Timer_getCrossTriggerConfig


/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerSrc
 *        获取跨定时器触发源
 */
#define DL_TimerA_getCrossTriggerSrc                 DL_Timer_getCrossTriggerSrc


/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerInputCond
 *        获取跨定时器触发输入条件
 */
#define DL_TimerA_getCrossTriggerInputCond     DL_Timer_getCrossTriggerInputCond

/*!
 * @brief Redirects to common @ref DL_Timer_getCrossTriggerEnable
 *        获取跨定时器触发使能状态
 */
#define DL_TimerA_getCrossTriggerEnable           DL_Timer_getCrossTriggerEnable


/*!
 * @brief Redirects to common @ref DL_Timer_enableShadowFeatures
 *        使能影子寄存器特性
 */
#define DL_TimerA_enableShadowFeatures             DL_Timer_enableShadowFeatures

/*!
 * @brief Redirects to common @ref DL_Timer_disableShadowFeatures
 *        禁用影子寄存器特性
 */
#define DL_TimerA_disableShadowFeatures           DL_Timer_disableShadowFeatures


/*!
 * @brief Redirects to common @ref DL_Timer_generateCrossTrigger
 *        产生跨定时器触发信号
 */
#define DL_TimerA_generateCrossTrigger             DL_Timer_generateCrossTrigger

/*!
 * @brief Redirects to common @ref DL_Timer_setLoadValue
 *        设置定时器装载值
 */
#define DL_TimerA_setLoadValue                             DL_Timer_setLoadValue

/*!
 * @brief Redirects to common @ref DL_Timer_getLoadValue
 *        获取定时器装载值
 */
#define DL_TimerA_getLoadValue                             DL_Timer_getLoadValue

/*!
 * @brief Redirects to common @ref DL_Timer_getTimerCount
 *        获取定时器当前计数值
 */
#define DL_TimerA_getTimerCount                           DL_Timer_getTimerCount

/*!
 * @brief Redirects to common @ref DL_Timer_setTimerCount
 *        设置定时器计数值
 */
#define DL_TimerA_setTimerCount                           DL_Timer_setTimerCount

/*!
 * @brief Redirects to common @ref DL_Timer_enableLZEventSuppression
 */
#define DL_TimerA_enableLZEventSuppression                     DL_Timer_enableLZEventSuppression

/*!
 * @brief Redirects to common @ref DL_Timer_disableLZEventSuppression
 */
#define DL_TimerA_disableLZEventSuppression                   DL_Timer_disableLZEventSuppression

/*!
 * @brief Redirects to common @ref DL_Timer_isLZEventSuppressionEnabled
 */
#define DL_TimerA_isLZEventSuppressionEnabled               DL_Timer_isLZEventSuppressionEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_setDebugReleaseBehavior
 */
#define DL_TimerA_setDebugReleaseBehavior       DL_Timer_setDebugReleaseBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_getDebugReleaseBehavior
 */
#define DL_TimerA_getDebugReleaseBehavior       DL_Timer_getDebugReleaseBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterControl
 */
#define DL_TimerA_setCounterControl                   DL_Timer_setCounterControl

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterZeroControl
 */
#define DL_TimerA_getCounterZeroControl           DL_Timer_getCounterZeroControl

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterAdvanceControl
 */
#define DL_TimerA_getCounterAdvanceControl     DL_Timer_getCounterAdvanceControl

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterLoadControl
 */
#define DL_TimerA_getCounterLoadControl           DL_Timer_getCounterLoadControl

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterMode
 */
#define DL_TimerA_setCounterMode                         DL_Timer_setCounterMode

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterMode
 */
#define DL_TimerA_getCounterMode                         DL_Timer_getCounterMode

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterValueAfterEnable
 */
#define DL_TimerA_setCounterValueAfterEnable DL_Timer_setCounterValueAfterEnable

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterValueAfterEnable
 */
#define DL_TimerA_getCounterValueAfterEnable DL_Timer_getCounterValueAfterEnable

/*!
 * @brief Redirects to common @ref DL_Timer_setCounterRepeatMode
 */
#define DL_TimerA_setCounterRepeatMode             DL_Timer_setCounterRepeatMode

/*!
 * @brief Redirects to common @ref DL_Timer_getCounterRepeatMode
 */
#define DL_TimerA_getCounterRepeatMode             DL_Timer_getCounterRepeatMode

/*!
 * @brief Redirects to common @ref DL_Timer_initTimerMode
 *        初始化定时器模式
 */
#define DL_TimerA_initTimerMode                           DL_Timer_initTimerMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCaptureMode
 *        初始化捕获模式
 */
#define DL_TimerA_initCaptureMode                       DL_Timer_initCaptureMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCaptureTriggerMode
 *        初始化捕获触发模式
 */
#define DL_TimerA_initCaptureTriggerMode         DL_Timer_initCaptureTriggerMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCaptureCombinedMode
 *        初始化组合捕获模式
 */
#define DL_TimerA_initCaptureCombinedMode       DL_Timer_initCaptureCombinedMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCompareMode
 *        初始化比较模式
 */
#define DL_TimerA_initCompareMode                       DL_Timer_initCompareMode

/*!
 * @brief Redirects to common @ref DL_Timer_initCompareTriggerMode
 *        初始化比较触发模式
 */
#define DL_TimerA_initCompareTriggerMode         DL_Timer_initCompareTriggerMode


/*!
 * @brief Redirects to common @ref DL_Timer_resetCounterMode
 *        复位计数器模式
 */
#define DL_TimerA_resetCounterMode                     DL_Timer_resetCounterMode

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareValue
 *        设置捕获比较值
 */
#define DL_TimerA_setCaptureCompareValue         DL_Timer_setCaptureCompareValue

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareValue
 *        获取捕获比较值
 */
#define DL_TimerA_getCaptureCompareValue         DL_Timer_getCaptureCompareValue

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareCtl
 *        设置捕获比较控制
 */
#define DL_TimerA_setCaptureCompareCtl             DL_Timer_setCaptureCompareCtl

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareCtl
 *        获取捕获比较控制
 */
#define DL_TimerA_getCaptureCompareCtl             DL_Timer_getCaptureCompareCtl

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompSrcDn
 *        设置第二比较源（向下计数）
 */
#define DL_TimerA_setSecondCompSrcDn                 DL_Timer_setSecondCompSrcDn

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompSrcDn
 *        获取第二比较源（向下计数）
 */
#define DL_TimerA_getSecondCompSrcDn                 DL_Timer_GetSecondCompSrcDn

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompSrcUp
 *        设置第二比较源（向上计数）
 */
#define DL_TimerA_setSecondCompSrcUp                 DL_Timer_setSecondCompSrcUp

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompSrcUp
 *        获取第二比较源（向上计数）
 */
#define DL_TimerA_getSecondCompSrcUp                 DL_Timer_GetSecondCompSrcUp

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptCompUpdateMethod
 *        设置捕获比较更新方式
 */
#define DL_TimerA_setCaptCompUpdateMethod          DL_Timer_setCaptCompUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptCompUpdateMethod
 *        获取捕获比较更新方式
 */
#define DL_TimerA_getCaptCompUpdateMethod          DL_Timer_getCaptCompUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptCompActUpdateMethod
 *        设置捕获比较动作更新方式
 */
#define DL_TimerA_setCaptCompActUpdateMethod          DL_Timer_setCaptCompActUpdateMethod

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptCompActUpdateMethod
 *        获取捕获比较动作更新方式
 */
#define DL_TimerA_getCaptCompActUpdateMethod          DL_Timer_getCaptCompActUpdateMethod


/*!
 * @brief Redirects to common @ref DL_Timer_enableSuppressionOfCompEvent
 */
#define DL_TimerA_enableSuppressionOfCompEvent                 DL_Timer_enableSuppressionOfCompEvent

/*!
 * @brief Redirects to common @ref DL_Timer_disableSuppressionOfCompEvent
 */
#define DL_Timer_disableSuppressionOfCompEvent                DL_Timer_disableSuppressionOfCompEvent

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareOutCtl
 *        设置捕获比较输出控制
 */
#define DL_TimerA_setCaptureCompareOutCtl       DL_Timer_setCaptureCompareOutCtl

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareOutCtl
 *        获取捕获比较输出控制
 */
#define DL_TimerA_getCaptureCompareOutCtl       DL_Timer_getCaptureCompareOutCtl

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareAction
 *        设置捕获比较动作
 */
#define DL_TimerA_setCaptureCompareAction       DL_Timer_setCaptureCompareAction

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareAction
 *        获取捕获比较动作
 */
#define DL_TimerA_getCaptureCompareAction       DL_Timer_getCaptureCompareAction


/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompActionDn
 */
#define DL_TimerA_setSecondCompActionDn           DL_Timer_setSecondCompActionDn

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompActionDn
 */
#define DL_TimerA_getSecondCompActionDn           DL_Timer_getSecondCompActionDn

/*!
 * @brief Redirects to common @ref DL_Timer_setSecondCompActionUp
 */
#define DL_TimerA_setSecondCompActionUp           DL_Timer_setSecondCompActionUp

/*!
 * @brief Redirects to common @ref DL_Timer_getSecondCompActionUp
 */
#define DL_TimerA_getSecondCompActionUp           DL_Timer_getSecondCompActionUp


/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareInput
 */
#define DL_TimerA_setCaptureCompareInput \
                                           DL_Timer_setCaptureCompareInput

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareInput
 */
#define DL_TimerA_getCaptureCompareInput \
                                           DL_Timer_getCaptureCompareInput

/*!
 * @brief Redirects to common @ref DL_Timer_setCaptureCompareInputFilter
 */
#define DL_TimerA_setCaptureCompareInputFilter \
                                           DL_Timer_setCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_getCaptureCompareInputFilter
 */
#define DL_TimerA_getCaptureCompareInputFilter \
                                           DL_Timer_getCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_enableCaptureCompareInputFilter
 */
#define DL_TimerA_enableCaptureCompareInputFilter \
                                        DL_Timer_enableCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_disableCaptureCompareInputFilter
 */
#define DL_TimerA_disableCaptureCompareInputFilter \
                                       DL_Timer_disableCaptureCompareInputFilter

/*!
 * @brief Redirects to common @ref DL_Timer_isCaptureCompareInputFilterEnabled
 */
#define DL_TimerA_isCaptureCompareInputFilterEnabled \
                                     DL_Timer_isCaptureCompareInputFilterEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_setExternalTriggerEvent
 */
#define DL_TimerA_setExternalTriggerEvent       DL_Timer_setExternalTriggerEvent

/*!
 * @brief Redirects to common @ref DL_Timer_getExternalTriggerEvent
 */
#define DL_TimerA_getExternalTriggerEvent       DL_Timer_getExternalTriggerEvent

/*!
 * @brief Redirects to common @ref DL_Timer_enableExternalTrigger
 */
#define DL_TimerA_enableExternalTrigger           DL_Timer_enableExternalTrigger

/*!
 * @brief Redirects to common @ref DL_Timer_disableExternalTrigger
 */
#define DL_TimerA_disableExternalTrigger         DL_Timer_disableExternalTrigger

/*!
 * @brief Redirects to common @ref DL_Timer_isExternalTriggerEnabled
 */
#define DL_TimerA_isExternalTriggerEnabled     DL_Timer_isExternalTriggerEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_startCounter
 *        启动定时器计数
 */
#define DL_TimerA_startCounter                             DL_Timer_startCounter

/*!
 * @brief Redirects to common @ref DL_Timer_stopCounter
 *        停止定时器计数
 */
#define DL_TimerA_stopCounter                               DL_Timer_stopCounter

/*!
 * @brief Redirects to common @ref DL_Timer_isRunning
 *        查询定时器是否正在运行
 */
#define DL_TimerA_isRunning                                   DL_Timer_isRunning

/*!
 * @brief Redirects to common @ref DL_Timer_enableInterrupt
 *        使能定时器中断
 */
#define DL_TimerA_enableInterrupt                       DL_Timer_enableInterrupt

/*!
 * @brief Redirects to common @ref DL_Timer_disableInterrupt
 *        禁用定时器中断
 */
#define DL_TimerA_disableInterrupt                     DL_Timer_disableInterrupt

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledInterrupts
 *        获取已使能的中断
 */
#define DL_TimerA_getEnabledInterrupts             DL_Timer_getEnabledInterrupts

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledInterruptStatus
 *        获取已使能中断的状态
 */
#define DL_TimerA_getEnabledInterruptStatus   DL_Timer_getEnabledInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_getRawInterruptStatus
 *        获取原始中断状态
 */
#define DL_TimerA_getRawInterruptStatus           DL_Timer_getRawInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_getPendingInterrupt
 *        获取待处理中断
 */
#define DL_TimerA_getPendingInterrupt               DL_Timer_getPendingInterrupt

/*!
 * @brief Redirects to common @ref DL_Timer_clearInterruptStatus
 *        清除中断状态
 */
#define DL_TimerA_clearInterruptStatus             DL_Timer_clearInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_setDeadBand
 *        设置死区时间
 */
#define DL_TimerA_setDeadBand                               DL_Timer_setDeadBand

/*!
 * @brief Redirects to common @ref DL_Timer_getDeadBandFallDelay
 *        获取死区下降沿延迟
 */
#define DL_TimerA_getDeadBandFallDelay             DL_Timer_getDeadBandFallDelay

/*!
 * @brief Redirects to common @ref DL_Timer_getDeadBandRiseDelay
 *        获取死区上升沿延迟
 */
#define DL_TimerA_getDeadBandRiseDelay             DL_Timer_getDeadBandRiseDelay

/*!
 * @brief Redirects to common @ref DL_Timer_setRepeatCounter
 *        设置重复计数器
 */
#define DL_TimerA_setRepeatCounter                     DL_Timer_setRepeatCounter

/*!
 * @brief Redirects to common @ref DL_Timer_getRepeatCounter
 *        获取重复计数器
 */
#define DL_TimerA_getRepeatCounter                     DL_Timer_getRepeatCounter

/*!
 * @brief Redirects to common @ref DL_Timer_enablePhaseLoad
 *        使能相位装载
 */
#define DL_TimerA_enablePhaseLoad                       DL_Timer_enablePhaseLoad

/*!
 * @brief Redirects to common @ref DL_Timer_disablePhaseLoad
 *        禁用相位装载
 */
#define DL_TimerA_disablePhaseLoad                     DL_Timer_disablePhaseLoad

/*!
 * @brief Redirects to common @ref DL_Timer_isPhaseLoadEnabled
 *        查询相位装载是否使能
 */
#define DL_TimerA_isPhaseLoadEnabled                 DL_Timer_isPhaseLoadEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_setPhaseLoadValue
 *        设置相位装载值
 */
#define DL_TimerA_setPhaseLoadValue                   DL_Timer_setPhaseLoadValue

/*!
 * @brief Redirects to common @ref DL_Timer_getPhaseLoadValue
 *        获取相位装载值
 */
#define DL_TimerA_getPhaseLoadValue                   DL_Timer_getPhaseLoadValue

/*!
 * @brief Redirects to common @ref DL_Timer_setPublisherChanID
 *        设置事件发布者通道 ID
 */
#define DL_TimerA_setPublisherChanID                 DL_Timer_setPublisherChanID

/*!
 * @brief Redirects to common @ref DL_Timer_getPublisherChanID
 *        获取事件发布者通道 ID
 */
#define DL_TimerA_getPublisherChanID                 DL_Timer_getPublisherChanID

/*!
 * @brief Redirects to common @ref DL_Timer_setSubscriberChanID
 *        设置事件订阅者通道 ID
 */
#define DL_TimerA_setSubscriberChanID               DL_Timer_setSubscriberChanID

/*!
 * @brief Redirects to common @ref DL_Timer_getSubscriberChanID
 *        获取事件订阅者通道 ID
 */
#define DL_TimerA_getSubscriberChanID               DL_Timer_getSubscriberChanID

/*!
 * @brief Redirects to common @ref DL_Timer_enableEvent
 *        使能定时器事件
 */
#define DL_TimerA_enableEvent                               DL_Timer_enableEvent

/*!
 * @brief Redirects to common @ref DL_Timer_disableEvent
 *        禁用定时器事件
 */
#define DL_TimerA_disableEvent                             DL_Timer_disableEvent

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledEvents
 *        获取已使能的事件
 */
#define DL_TimerA_getEnabledEvents                     DL_Timer_getEnabledEvents

/*!
 * @brief Redirects to common @ref DL_Timer_getEnabledEventStatus
 *        获取已使能事件的状态
 */
#define DL_TimerA_getEnabledEventStatus           DL_Timer_getEnabledEventStatus

/*!
 * @brief Redirects to common @ref DL_Timer_getRawEventsStatus
 *        获取原始事件状态
 */
#define DL_TimerA_getRawEventsStatus                 DL_Timer_getRawEventsStatus

/*!
 * @brief Redirects to common @ref DL_Timer_clearInterruptStatus
 *        清除事件状态
 */
#define DL_TimerA_clearInterruptStatus             DL_Timer_clearInterruptStatus

/*!
 * @brief Redirects to common @ref DL_Timer_setFaultConfig
 *        设置故障配置
 */
#define DL_TimerA_setFaultConfig                         DL_Timer_setFaultConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getFaultConfig
 *        获取故障配置
 */
#define DL_TimerA_getFaultConfig                         DL_Timer_getFaultConfig

/*!
 * @brief Redirects to common @ref DL_Timer_enableFaultInput
 *        使能故障输入
 */
#define DL_TimerA_enableFaultInput                     DL_Timer_enableFaultInput

/*!
 * @brief Redirects to common @ref DL_Timer_disableFaultInput
 *        禁用故障输入
 */
#define DL_TimerA_disableFaultInput                   DL_Timer_disableFaultInput

/*!
 * @brief Redirects to common @ref DL_Timer_isFaultInputEnabled
 *        查询故障输入是否使能
 */
#define DL_TimerA_isFaultInputEnabled               DL_Timer_isFaultInputEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_enableClockFaultDetection
 *        使能时钟故障检测
 */
#define DL_TimerA_enableClockFaultDetection   DL_Timer_enableClockFaultDetection

/*!
 * @brief Redirects to common @ref DL_Timer_disableClockFaultDetection
 *        禁用时钟故障检测
 */
#define DL_TimerA_disableClockFaultDetection \
                                             DL_Timer_disableClockFaultDetection

/*!
 * @brief Redirects to common @ref DL_Timer_isClockFaultDetectionEnabled
 *        查询时钟故障检测是否使能
 */
#define DL_TimerA_isClockFaultDetectionEnabled \
                                           DL_Timer_isClockFaultDetectionEnabled

/*!
 * @brief Redirects to common @ref DL_Timer_setFaultSourceConfig
 *        设置故障源配置
 */
#define DL_TimerA_setFaultSourceConfig             DL_Timer_setFaultSourceConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getFaultSourceConfig
 *        获取故障源配置
 */
#define DL_TimerA_getFaultSourceConfig             DL_Timer_getFaultSourceConfig

/*!
 * @brief Redirects to common @ref DL_Timer_setFaultInputFilterConfig
 *        设置故障输入滤波配置
 */
#define DL_TimerA_setFaultInputFilterConfig   DL_Timer_setFaultInputFilterConfig

/*!
 * @brief Redirects to common @ref DL_Timer_getFaultInputFilterConfig
 *        获取故障输入滤波配置
 */
#define DL_TimerA_getFaultInputFilterConfig   DL_Timer_getFaultInputFilterConfig

/*!
 * @brief Redirects to common @ref DL_Timer_configFaultOutputAction
 *        配置故障输出动作
 */
#define DL_TimerA_configFaultOutputAction       DL_Timer_configFaultOutputAction

/*!
 * @brief Redirects to common @ref DL_Timer_configFaultCounter
 *        配置故障计数器
 */
#define DL_TimerA_configFaultCounter                 DL_Timer_configFaultCounter

/*!
 * @brief Redirects to common @ref DL_Timer_setCoreHaltBehavior
 *        设置内核暂停时定时器行为
 */
#define DL_TimerA_setCoreHaltBehavior               DL_Timer_setCoreHaltBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_getCoreHaltBehavior
 *        获取内核暂停时定时器行为
 */
#define DL_TimerA_getHaltBehavior                   DL_Timer_getCoreHaltBehavior

/*!
 * @brief Redirects to common @ref DL_Timer_initFourCCPWMMode
 *        初始化四通道 PWM 模式
 */
#define DL_TimerA_initPWMMode                   DL_Timer_initFourCCPWMMode

/* clang-format on */

/**
 *  @brief      Saves TimerA configuration before entering STOP or STANDBY mode.
 *              Timer must be in IDLE state before calling this API. Timer can
 *              be put IDLE state by calling @ref DL_TimerA_stopCounter.
 *              进入 STOP 或 STANDBY 模式前保存 TimerA 配置。
 *              调用前定时器必须处于 IDLE 状态。
 *
 *  @param[in]  gptimer  Pointer to the register overlay for the peripheral
 *                       外设寄存器基地址指针
 *
 *  @param[in]  ptr      Configuration backup setup structure. See
 *                       @ref DL_TimerA_backupConfig.
 *                       配置备份结构体指针
 *
 *  @retval     FALSE if a configuration already exists in ptr (will not be
 *              overwritten). TRUE if a configuration was successfully saved
 *              FALSE 表示 ptr 中已有配置（不会覆盖），TRUE 表示保存成功
 *
 */
bool DL_TimerA_saveConfiguration(
    const GPTIMER_Regs *gptimer, DL_TimerA_backupConfig *ptr);

/**
 *  @brief      Restore TimerA configuration after leaving a STOP or STANDBY
 *              mode.
 *              退出 STOP 或 STANDBY 模式后恢复 TimerA 配置。
 *
 *  @param[in]  gptimer  Pointer to the register overlay for the peripheral
 *                       外设寄存器基地址指针
 *
 *  @param[in]  ptr      Configuration backup setup structure. See
 *                       @ref DL_TimerA_backupConfig.
 *                       配置备份结构体指针
 *  @param[in]  restoreCounter If true timer counter value is also
 *                             restored (e.g. resume counts of edge counts).
 *                             If false timer counter is not restored.
 *                             为 true 时同时恢复计数器值，为 false 时不恢复
 *
 *  @retval     FALSE if a configuration does not exist in ptr (will not be
 *              loaded). TRUE if a configuration successfully loaded
 *              FALSE 表示 ptr 中无配置（不会加载），TRUE 表示加载成功
 *
 */
bool DL_TimerA_restoreConfiguration(
    GPTIMER_Regs *gptimer, DL_TimerA_backupConfig *ptr, bool restoreCounter);

#ifdef __cplusplus
}
#endif

#endif /* __MSPM0_HAS_TIMER_A__ */

#endif /* ti_dl_dl_timera__include */
/** @}*/
