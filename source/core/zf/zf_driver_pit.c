/*****************************************

* 文件名称          zf_driver_pit

****************************************/

#include "zf_common_clock.h"
#include "zf_common_debug.h"
#include "zf_common_interrupt.h"
#include "zf_driver_timer.h"

#include "zf_driver_pit.h"

static void pit_callbakc_defalut(uint32 event, void *ptr);
void_callback_uint32_ptr pit_callback_list[PIT_NUM] =
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL};
void *pit_callback_ptr_list[PIT_NUM] =
    {
        pit_callbakc_defalut, pit_callbakc_defalut, pit_callbakc_defalut, pit_callbakc_defalut,
        pit_callbakc_defalut, pit_callbakc_defalut, pit_callbakc_defalut};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PIT 中断默认回调函数
// 参数说明     event               触发中断的事件 此处默认为 0
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static void pit_callbakc_defalut(uint32 event, void *ptr)
{
    (void)event;
    (void)ptr;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PIT 中断使能
// 参数说明     pit_n           PIT 外设模块号
// 返回参数     void
// 使用示例     pit_enable(pit_n);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_enable(pit_index_enum pit_n)
{
    switch (pit_n)
    {
    case PIT_TIM_A0:
        TIMA0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    case PIT_TIM_A1:
        TIMA1->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    case PIT_TIM_G0:
        TIMG0->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    case PIT_TIM_G6:
        TIMG6->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    case PIT_TIM_G7:
        TIMG7->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    case PIT_TIM_G8:
        TIMG8->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    case PIT_TIM_G12:
        TIMG12->COUNTERREGS.CTRCTL |= GPTIMER_CTRCTL_EN_ENABLED;
        break;
    default:
        break;
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     PIT 中断禁止
// 参数说明     pit_n           PIT 外设模块号
// 返回参数     void
// 使用示例     pit_disable(pit_n);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_disable(pit_index_enum pit_n)
{
    switch (pit_n)
    {
    case PIT_TIM_A0:
        TIMA0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    case PIT_TIM_A1:
        TIMA1->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    case PIT_TIM_G0:
        TIMG0->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    case PIT_TIM_G6:
        TIMG6->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    case PIT_TIM_G7:
        TIMG7->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    case PIT_TIM_G8:
        TIMG8->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    case PIT_TIM_G12:
        TIMG12->COUNTERREGS.CTRCTL &= ~GPTIMER_CTRCTL_EN_MASK;
        break;
    default:
        break;
    }
}

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
void pit_init(pit_index_enum pit_n, uint32 period, void_callback_uint32_ptr callback, void *ptr)
{
    // 如果程序在输出了断言信息 并且提示出错位置在这里
    // 就去查看你在什么地方调用这个函数 检查你的传入参数
    // 这里是检查是否有重复使用定时器
    // 比如初始化了 TIM1_PWM 然后又初始化成 TIM1_PIT 这种用法是不允许的
    zf_assert(timer_funciton_check((timer_index_enum)pit_n, TIMER_FUNCTION_TIMER));
    // 如果是这一行报错 那我就得问问你为什么周期写的是 0
    zf_assert(0 != period);
    zf_assert(0x7FF0 >= period);

    timer_clock_enable((timer_index_enum)pit_n); // 使能时钟

    pit_callback_ptr_list[pit_n] = ptr;
    pit_callback_list[pit_n] = (NULL == callback) ? (pit_callbakc_defalut) : (callback);

    GPTIMER_Regs *timer_obj;
    switch (pit_n)
    {
    case PIT_TIM_A0:
        timer_obj = TIMA0;
        break;
    case PIT_TIM_A1:
        timer_obj = TIMA1;
        break;
    case PIT_TIM_G0:
        timer_obj = TIMG0;
        break;
    case PIT_TIM_G6:
        timer_obj = TIMG6;
        break;
    case PIT_TIM_G7:
        timer_obj = TIMG7;
        break;
    case PIT_TIM_G8:
        timer_obj = TIMG8;
        break;
    case PIT_TIM_G12:
        timer_obj = TIMG12;
        break;
    default:
        break;
    }

    timer_obj->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_CM_UP | GPTIMER_CTRCTL_REPEAT_REPEAT_1;
    timer_obj->CLKSEL = GPTIMER_CLKSEL_BUSCLK_SEL_ENABLE;
    if (TIM_G0 == pit_n || TIM_G8 == pit_n)
    {
        timer_obj->CLKDIV = 0;
        timer_obj->COMMONREGS.CPS = 0;
    }
    else
    {
        timer_obj->CLKDIV = 1;
        timer_obj->COMMONREGS.CPS = 0;
    }
    timer_obj->COUNTERREGS.LOAD = period;
    timer_obj->CPU_INT.IMASK |= GPTIMER_CPU_INT_IMASK_L_SET;

    const IRQn_Type irq_index[PIT_NUM] = {
        TIMA0_INT_IRQn, TIMA1_INT_IRQn, TIMG0_INT_IRQn, TIMG6_INT_IRQn,
        TIMG7_INT_IRQn, TIMG8_INT_IRQn, TIMG12_INT_IRQn};
    interrupt_enable(irq_index[(pit_n)]);

    pit_enable(pit_n);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIM PIT 中断初始化 us 周期
// 参数说明     pit_n               使用的 PIT 编号
// 参数说明     us                  PIT 周期 us 级别
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     pit_us_init(pit_n, period, callback, NULL);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_us_init(pit_index_enum pit_n, uint32 period, void_callback_uint32_ptr callback, void *ptr)
{
    // 如果程序在输出了断言信息 并且提示出错位置在这里
    // 就去查看你在什么地方调用这个函数 检查你的传入参数
    // 这里是检查是否有重复使用定时器
    // 比如初始化了 TIM1_PWM 然后又初始化成 TIM1_PIT 这种用法是不允许的
    zf_assert(timer_funciton_check((timer_index_enum)pit_n, TIMER_FUNCTION_TIMER));
    // 如果是这一行报错 那我就得问问你为什么周期写的是 0
    zf_assert(0 != period);
    zf_assert(0x7FF0 >= period);

    timer_clock_enable((timer_index_enum)pit_n); // 使能时钟

    pit_callback_ptr_list[pit_n] = ptr;
    pit_callback_list[pit_n] = (NULL == callback) ? (pit_callbakc_defalut) : (callback);

    GPTIMER_Regs *timer_obj;
    switch (pit_n)
    {
    case PIT_TIM_A0:
        timer_obj = TIMA0;
        break;
    case PIT_TIM_A1:
        timer_obj = TIMA1;
        break;
    case PIT_TIM_G0:
        timer_obj = TIMG0;
        break;
    case PIT_TIM_G6:
        timer_obj = TIMG6;
        break;
    case PIT_TIM_G7:
        timer_obj = TIMG7;
        break;
    case PIT_TIM_G8:
        timer_obj = TIMG8;
        break;
    case PIT_TIM_G12:
        timer_obj = TIMG12;
        break;
    default:
        break;
    }

    timer_obj->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_CM_UP | GPTIMER_CTRCTL_REPEAT_REPEAT_1;
    ;
    timer_obj->CLKSEL = GPTIMER_CLKSEL_MFCLK_SEL_ENABLE;
    timer_obj->CLKDIV = 3;
    timer_obj->COMMONREGS.CPS = 0;
    timer_obj->COUNTERREGS.LOAD = period;
    timer_obj->CPU_INT.IMASK |= GPTIMER_CPU_INT_IMASK_L_SET;

    const IRQn_Type irq_index[PIT_NUM] = {
        TIMA0_INT_IRQn, TIMA1_INT_IRQn, TIMG0_INT_IRQn, TIMG6_INT_IRQn,
        TIMG7_INT_IRQn, TIMG8_INT_IRQn, TIMG12_INT_IRQn};
    interrupt_enable(irq_index[(pit_n)]);

    pit_enable(pit_n);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     TIM PIT 中断初始化 ms 周期
// 参数说明     pit_n               使用的 PIT 编号
// 参数说明     ms                  PIT 周期 ms 级别
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     pit_ms_init(pit_n, period, callback, NULL);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void pit_ms_init(pit_index_enum pit_n, uint32 period, void_callback_uint32_ptr callback, void *ptr)
{
    // 如果程序在输出了断言信息 并且提示出错位置在这里
    // 就去查看你在什么地方调用这个函数 检查你的传入参数
    // 这里是检查是否有重复使用定时器
    // 比如初始化了 TIM1_PWM 然后又初始化成 TIM1_PIT 这种用法是不允许的
    zf_assert(timer_funciton_check((timer_index_enum)pit_n, TIMER_FUNCTION_TIMER));
    // 如果是这一行报错 那我就得问问你为什么周期写的是 0
    zf_assert(0 != period);
    zf_assert(0x7FF0 >= period);

    timer_clock_enable((timer_index_enum)pit_n); // 使能时钟

    pit_callback_ptr_list[pit_n] = ptr;
    pit_callback_list[pit_n] = (NULL == callback) ? (pit_callbakc_defalut) : (callback);

    GPTIMER_Regs *timer_obj;
    switch (pit_n)
    {
    case PIT_TIM_A0:
        timer_obj = TIMA0;
        break;
    case PIT_TIM_A1:
        timer_obj = TIMA1;
        break;
    case PIT_TIM_G0:
        timer_obj = TIMG0;
        break;
    case PIT_TIM_G6:
        timer_obj = TIMG6;
        break;
    case PIT_TIM_G7:
        timer_obj = TIMG7;
        break;
    case PIT_TIM_G8:
        timer_obj = TIMG8;
        break;
    case PIT_TIM_G12:
        timer_obj = TIMG12;
        break;
    default:
        break;
    }

    timer_obj->COUNTERREGS.CTRCTL = GPTIMER_CTRCTL_CM_UP | GPTIMER_CTRCTL_REPEAT_REPEAT_1;
    timer_obj->CLKSEL = GPTIMER_CLKSEL_MFCLK_SEL_ENABLE;
    timer_obj->CLKDIV = 7;
    if (period <= 20)
    {
        switch (period)
        {
        case 1:
            timer_obj->COMMONREGS.CPS = 180;
            break;
        case 2:
            timer_obj->COMMONREGS.CPS = 203;
            break;
        case 3:
            timer_obj->COMMONREGS.CPS = 220;
            break;
        case 4:
            timer_obj->COMMONREGS.CPS = 225;
            break;
        case 5:
            timer_obj->COMMONREGS.CPS = 229;
            break;
        case 6:
            timer_obj->COMMONREGS.CPS = 233;
            break;
        case 7:
            timer_obj->COMMONREGS.CPS = 236;
            break;
        case 8:
            timer_obj->COMMONREGS.CPS = 239;
            break;
        case 9:
            timer_obj->COMMONREGS.CPS = 240;
            break;
        case 10:
            timer_obj->COMMONREGS.CPS = 241;
            break;
        case 11:
            timer_obj->COMMONREGS.CPS = 242;
            break;
        case 12:
            timer_obj->COMMONREGS.CPS = 242;
            break;
        case 13:
            timer_obj->COMMONREGS.CPS = 244;
            break;
        case 14:
            timer_obj->COMMONREGS.CPS = 245;
            break;
        case 15:
            timer_obj->COMMONREGS.CPS = 245;
            break;
        case 16:
            timer_obj->COMMONREGS.CPS = 245;
            break;
        case 17:
            timer_obj->COMMONREGS.CPS = 246;
            break;
        case 18:
            timer_obj->COMMONREGS.CPS = 246;
            break;
        case 19:
            timer_obj->COMMONREGS.CPS = 246;
            break;
        case 20:
            timer_obj->COMMONREGS.CPS = 247;
            break;
        default:
            break;
        }
    }
    else
    {
        timer_obj->COMMONREGS.CPS = 250;
    }

    if (PIT_TIM_G12 == pit_n)
    {
        timer_obj->COUNTERREGS.LOAD = period * 2 * 250;
    }
    else
    {
        timer_obj->COUNTERREGS.LOAD = period * 2;
    }
    timer_obj->CPU_INT.IMASK |= GPTIMER_CPU_INT_IMASK_L_SET;

    const IRQn_Type irq_index[PIT_NUM] = {
        TIMA0_INT_IRQn, TIMA1_INT_IRQn, TIMG0_INT_IRQn, TIMG6_INT_IRQn,
        TIMG7_INT_IRQn, TIMG8_INT_IRQn, TIMG12_INT_IRQn};
    interrupt_enable(irq_index[(pit_n)]);

    pit_enable(pit_n);
}
