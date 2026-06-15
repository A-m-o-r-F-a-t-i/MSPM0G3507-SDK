/*****************************************

* 文件名称          zf_driver_delay

****************************************/

#include "zf_driver_delay.h"

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     system 延时函数 ms 级别
// 参数说明     time        需要延时的时间 ms 级别
// 返回参数     void
// 使用示例     system_delay_ms(100);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void system_delay_ms(uint32 time)
{
    SysTick->LOAD = CPUCLK_FREQ / 1000;
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (time--)
    {
        while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
            ;
        SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     system 延时函数 us 级别
// 参数说明     time        需要延时的时间 us 级别
// 返回参数     void
// 使用示例     system_delay_us(100);
// 备注信息     受限于程序运行跳转 此延时会比输入值高出一些
//-------------------------------------------------------------------------------------------------------------------
void system_delay_us(uint32 time)
{
    SysTick->LOAD = CPUCLK_FREQ / 1000000 * time;
    SysTick->VAL = 0UL;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk))
        ;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     system delay 初始化
// 参数说明     void
// 返回参数     void
// 使用示例     system_delay_init();
// 备注信息     本函数由 clock_init 内部调用
//-------------------------------------------------------------------------------------------------------------------
void system_delay_init(void)
{
}
