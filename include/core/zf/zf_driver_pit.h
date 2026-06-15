/*****************************************

* 文件名称          zf_driver_pit
* 文件作用          周期中断定时器驱动（定时中断回调、多定时器管理）

****************************************/

#ifndef _zf_driver_pit_h_
#define _zf_driver_pit_h_

#include "zf_common_typedef.h"

#include "zf_driver_timer.h"

typedef enum // 枚举 PIT
{
    PIT_TIM_A0,
    PIT_TIM_A1,
    PIT_TIM_G0,
    PIT_TIM_G6,
    PIT_TIM_G7,
    PIT_TIM_G8,
    PIT_TIM_G12,
} pit_index_enum;

#define PIT_NUM (7)

extern void_callback_uint32_ptr pit_callback_list[PIT_NUM];
extern void *pit_callback_ptr_list[PIT_NUM];

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PIT 中断使能
// 参数说明     pit_n               PIT 外设模块号
// 返回参数     void
// 使用示例     pit_enable(pit_n);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_enable(pit_index_enum pit_n);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PIT 中断禁止
// 参数说明     pit_n               PIT 外设模块号
// 返回参数     void
// 使用示例     pit_disable(pit_n);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_disable(pit_index_enum pit_n);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PIT 初始化 一般调用 pit_ms_init 或 pit_us_init
// 参数说明     pit_n               PIT 外设模块号
// 参数说明     period              PIT 周期 一般是芯片或者模块时钟频率计算
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     pit_init(pit_n, period, callback, NULL);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_init(pit_index_enum pit_n, uint32 period, void_callback_uint32_ptr callback, void *ptr);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIM PIT 中断初始化 us 周期
// 参数说明     pit_n               使用的 PIT 编号
// 参数说明     period              PIT 周期 us 级别
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     pit_us_init(pit_n, period, callback, NULL);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_us_init(pit_index_enum pit_n, uint32 period, void_callback_uint32_ptr callback, void *ptr);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIM PIT 中断初始化 ms 周期
// 参数说明     pit_n               使用的 PIT 编号
// 参数说明     period              PIT 周期 ms 级别
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     pit_ms_init(pit_n, period, callback, NULL);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_ms_init(pit_index_enum pit_n, uint32 period, void_callback_uint32_ptr callback, void *ptr);

#endif
