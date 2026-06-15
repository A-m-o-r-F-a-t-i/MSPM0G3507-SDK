/*****************************************

* 文件名称          zf_common_clock
* 文件作用          系统时钟管理（晶振配置、时钟初始化、全局时钟变量）

****************************************/

#ifndef _zf_common_clock_h_
#define _zf_common_clock_h_

#include "zf_common_typedef.h"

#define BOARD_XTAL_FREQ (40000000)    // 晶振频率 如果自己用的不是这个频率就修改这里 UM 定义范围为 4-24Mhz
#define XTAL_STARTUP_TIMEOUT (0x0F00) // 晶振就绪等待超时时长

typedef enum
{
    SYSTEM_CLOCK_80M = 80000000, // 80Mhz
} system_clock_enum;

extern uint32 system_clock; // 全局变量 系统时钟信息 SYSTICK AHB
extern uint32 bus_clock;    // 全局变量 总线时钟信息 APB1 APB2

void clock_init(uint32 clock); // 核心时钟初始化

#endif
