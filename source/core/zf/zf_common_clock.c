/*****************************************

* 文件名称          zf_common_clock

****************************************/

#include "zf_common_function.h"
#include "zf_common_interrupt.h"

#include "zf_common_clock.h"

uint32 system_clock = SYSTEM_CLOCK_80M;  // 系统时钟信息
uint32 bus_clock = SYSTEM_CLOCK_80M / 2; // 总线时钟信息
extern void system_delay_init(void);
//-------------------------------------------------------------------------------------------------------------------
// 函数简介     核心时钟恢复初始设置
// 参数说明     void
// 返回参数     void
// 使用示例     clock_reset();
// 备注信息     本函数在文件内部调用 用户不用关注 也不可修改
//-------------------------------------------------------------------------------------------------------------------
static void clock_reset(void)
{
    DL_GPIO_reset(GPIOA);
    DL_GPIO_reset(GPIOB);
    DL_GPIO_enablePower(GPIOA);
    DL_GPIO_enablePower(GPIOB);

    DL_TimerA_reset(TIMA0);
    DL_TimerA_reset(TIMA1);
    DL_TimerA_enablePower(TIMA0);
    DL_TimerA_enablePower(TIMA1);

    DL_TimerG_reset(TIMG0);
    DL_TimerG_reset(TIMG6);
    DL_TimerG_reset(TIMG7);
    DL_TimerG_reset(TIMG8);
    DL_TimerG_reset(TIMG12);
    DL_TimerG_enablePower(TIMG0);
    DL_TimerG_enablePower(TIMG6);
    DL_TimerG_enablePower(TIMG7);
    DL_TimerG_enablePower(TIMG8);
    DL_TimerG_enablePower(TIMG12);

    DL_UART_Main_reset(UART0);
    DL_UART_Main_reset(UART1);
    DL_UART_Main_reset(UART2);
    DL_UART_Main_reset(UART3);
    DL_UART_Main_enablePower(UART0);
    DL_UART_Main_enablePower(UART1);
    DL_UART_Main_enablePower(UART2);
    DL_UART_Main_enablePower(UART3);

    DL_SPI_reset(SPI0);
    DL_SPI_reset(SPI1);
    DL_SPI_enablePower(SPI0);
    DL_SPI_enablePower(SPI1);

    DL_ADC12_reset(ADC0);
    DL_ADC12_reset(ADC1);
    DL_ADC12_enablePower(ADC0);
    DL_ADC12_enablePower(ADC1);

    DL_VREF_reset(VREF);
    DL_VREF_enablePower(VREF);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     核心时钟设置
// 参数说明     clock       时钟频率 推荐使用 zf_common_clock.h 中 system_clock_enum 定义的选项
// 返回参数     void
// 使用示例     clock_set_freq(clock);
// 备注信息     本函数在文件内部调用 用户不用关注 也不可修改
//-------------------------------------------------------------------------------------------------------------------
static void clock_set_freq(uint32 clock)
{
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     核心时钟初始化
// 参数说明     clock       时钟频率 推荐使用 zf_common_clock.h 中 system_clock_enum 定义的选项
// 返回参数     void
// 使用示例     clock_init(SYSTEM_CLOCK_80M);
//-------------------------------------------------------------------------------------------------------------------
void clock_init(uint32 clock)
{
    clock_reset();
    SYSCFG_DL_init();
    interrupt_init();
    system_delay_init();
}
