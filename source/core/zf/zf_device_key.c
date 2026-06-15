/*****************************************

* 文件名称          zf_device_key

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

#include "zf_common_debug.h"

#include "zf_device_key.h"

static uint32 scanner_period = 0;            // 按键的扫描周期
static uint32 key_press_time[KEY_NUMBER];    // 按键信号持续时长
static key_state_enum key_state[KEY_NUMBER]; // 按键状态

static const gpio_pin_enum key_index[KEY_NUMBER] = KEY_LIST;

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     按键状态扫描
// 参数说明     void
// 返回参数     void
// 使用示例     key_scanner();
// 备注信息     这个函数放在主循环或者 PIT 中断中
//-------------------------------------------------------------------------------------------------------------------
void key_scanner(void)
{
    uint8 i = 0;
    for (i = 0; KEY_NUMBER > i; i++)
    {
        if (KEY_RELEASE_LEVEL != gpio_get_level(key_index[i])) // 按键按下
        {
            key_press_time[i] += scanner_period;
            if (KEY_LONG_PRESS_PERIOD <= key_press_time[i])
            {
                key_state[i] = KEY_LONG_PRESS;
            }
        }
        else // 按键释放
        {
            if ((KEY_LONG_PRESS != key_state[i]) && (KEY_MAX_SHOCK_PERIOD <= key_press_time[i]) && (KEY_LONG_PRESS_PERIOD > key_press_time[i]))
            {
                key_state[i] = KEY_SHORT_PRESS;
            }
            else
            {
                key_state[i] = KEY_RELEASE;
            }
            key_press_time[i] = 0;
        }
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     获取按键状态
// 参数说明     key_n           按键索引
// 返回参数     key_state_enum  按键状态
// 使用示例     key_get_state(KEY_1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
key_state_enum key_get_state(key_index_enum key_n)
{
    return key_state[key_n];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     清除对应按键状态
// 参数说明     key_n           按键索引
// 返回参数     void            无
// 使用示例     key_clear_state(KEY_1);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void key_clear_state(key_index_enum key_n)
{
    key_state[key_n] = KEY_RELEASE;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     清除所有按键状态
// 参数说明     void            无
// 返回参数     void            无
// 使用示例     key_clear_all_state();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void key_clear_all_state(void)
{
    key_state[0] = KEY_RELEASE;
    key_state[1] = KEY_RELEASE;
    key_state[2] = KEY_RELEASE;
    key_state[3] = KEY_RELEASE;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     按键初始化
// 参数说明     period          按键扫描周期 以毫秒为单位
// 返回参数     void
// 使用示例     key_init(10);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void key_init(uint32 period)
{
    zf_assert(0 < period);
    uint8 loop_temp = 0;
    for (loop_temp = 0; KEY_NUMBER > loop_temp; loop_temp++)
    {
        gpio_init(key_index[loop_temp], GPI, GPIO_HIGH, GPI_PULL_UP);
        key_state[loop_temp] = KEY_RELEASE;
    }
    scanner_period = period;
}