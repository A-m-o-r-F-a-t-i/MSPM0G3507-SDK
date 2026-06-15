/*****************************************

* 文件名称          zf_device_key
* 文件作用          按键处理驱动（多按键扫描、消抖、长按/短按检测）

****************************************/
/*****************************************
 * 接线定义：
 *                   ------------------------------------
 *                   模块管脚            单片机管脚
 *                   // 一般是主板按键对应的引脚
 *                   KEY1/S1             查看 zf_device_key.h 中 KEY_LIST[0]
 *                   KEY2/S2             查看 zf_device_key.h 中 KEY_LIST[1]
 *                   KEY3/S3             查看 zf_device_key.h 中 KEY_LIST[2]
 *                   KEY4/S4             查看 zf_device_key.h 中 KEY_LIST[3]
 *                   ------------------------------------
 ****************************************/

#ifndef _zf_device_key_h_
#define _zf_device_key_h_

#include "zf_common_debug.h"

#include "zf_driver_gpio.h"

// 定义按键引脚 用户可以新增可以修改 默认定义四个按键
// 定义按键顺序对应下方 key_index_enum 枚举体中定义的顺序
// 如果用户可以新增按键 那么需要同步在下方 key_index_enum 枚举体中新增按键
#define KEY_LIST {A30, A31, B0, B1}

#define KEY_RELEASE_LEVEL (GPIO_HIGH) // 按键的默认状态 也就是按键释放状态的电平
#define KEY_MAX_SHOCK_PERIOD (10)     // 按键消抖检测时长 单位毫秒 低于这个时长的信号会被认为是杂波抖动
#define KEY_LONG_PRESS_PERIOD (1000)  // 最小长按时长 单位毫秒 高于这个时长的信号会被认为是长按动作

typedef enum
{
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_NUMBER,
} key_index_enum; // 按键索引 对应上方定义的按键引脚个数 默认定义四个按键

typedef enum
{
    KEY_RELEASE,     // 按键释放状态
    KEY_SHORT_PRESS, // 按键短按状态
    KEY_LONG_PRESS,  // 按键长按状态
} key_state_enum;

void key_scanner(void);
key_state_enum key_get_state(key_index_enum key_n);
void key_clear_state(key_index_enum key_n);
void key_clear_all_state(void);
void key_init(uint32 period);

#endif