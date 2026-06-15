/*****************************************

* 文件名称          zf_driver_exti
* 文件作用          外部中断驱动（GPIO 边沿触发中断、回调函数管理）

****************************************/

#ifndef _zf_driver_exti_h_
#define _zf_driver_exti_h_

#include "ti_msp_dl_config.h"

#include "zf_driver_gpio.h"

typedef enum
{                            // 枚举 EXTI 触发信号  此枚举定义不允许用户修改
    EXTI_TRIGGER_RISING = 1, // 上升沿触发模式
    EXTI_TRIGGER_FALLING,    // 下降沿触发模式
    EXTI_TRIGGER_BOTH,       // 双边沿触发模式
} exti_trigger_enum;

extern void_callback_uint32_ptr exti_callback_list[GPIO_GROUP_PIN_NUMBER_MAX];
extern void *exti_callback_ptr_list[GPIO_GROUP_PIN_NUMBER_MAX];

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断使能
// 参数说明     pin                 选择 EXTI 引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     exti_enable(pin);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_enable(gpio_pin_enum pin);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断失能
// 参数说明     pin                 选择 EXTI 引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     exti_disable(pin);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_disable(gpio_pin_enum pin);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断初始化
// 参数说明     pin                 选择 EXTI 引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 参数说明     trigger             选择触发的信号方式 [EXTI_TRIGGER_RISING/EXTI_TRIGGER_FALLING/EXTI_TRIGGER_BOTH]
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     exti_init(pin, trigger, callback, ptrs);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_init(gpio_pin_enum pin, exti_trigger_enum trigger, void_callback_uint32_ptr callback, void *ptr);

#endif
