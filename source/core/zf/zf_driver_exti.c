/*****************************************

* 文件名称          zf_driver_exti

****************************************/

#include "zf_common_debug.h"
#include "zf_common_interrupt.h"

#include "zf_driver_exti.h"

static void exti_callbakc_defalut(uint32 event, void *ptr);
void_callback_uint32_ptr exti_callback_list[GPIO_GROUP_PIN_NUMBER_MAX] =
    {
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut,
        exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut, exti_callbakc_defalut};
void *exti_callback_ptr_list[GPIO_GROUP_PIN_NUMBER_MAX] =
    {
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL};

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断默认回调函数
// 参数说明     event               触发中断的事件 此处默认为 0
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
static void exti_callbakc_defalut(uint32 event, void *ptr)
{
    (void)event;
    (void)ptr;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断使能
// 参数说明     pin                 选择 EXTI 引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     exti_enable(pin);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_enable(gpio_pin_enum pin)
{
    uint8 io_group = ((pin >> GPIO_GROUP_INDEX_OFFSET) & GPIO_GROUP_INDEX_MASK);
    uint8 io_pin = ((pin >> GPIO_PIN_INDEX_OFFSET) & GPIO_PIN_INDEX_MASK);
    gpio_group[io_group]->CPU_INT.IMASK |= (0x00000001 << io_pin);
    interrupt_enable(GPIOA_INT_IRQn);
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断失能
// 参数说明     pin                 选择 EXTI 引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 返回参数     void
// 使用示例     exti_disable(pin);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void exti_disable(gpio_pin_enum pin)
{
    uint8 io_group = ((pin >> GPIO_GROUP_INDEX_OFFSET) & GPIO_GROUP_INDEX_MASK);
    uint8 io_pin = ((pin >> GPIO_PIN_INDEX_OFFSET) & GPIO_PIN_INDEX_MASK);
    gpio_group[io_group]->CPU_INT.IMASK &= ~(0x00000001 << io_pin);

    if (!gpio_group[io_group]->CPU_INT.IMASK)
    {
        interrupt_disable(GPIOA_INT_IRQn);
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     EXTI 中断初始化
// 参数说明     pin                 选择 EXTI 引脚 (可选择范围由 zf_driver_gpio.h 内 gpio_pin_enum 枚举值确定)
// 参数说明     trigger             选择触发的信号方式 [EXTI_TRIGGER_RISING/EXTI_TRIGGER_FALLING/EXTI_TRIGGER_BOTH]
// 参数说明     callback            回调函数 为无返回值 uint32 加 void * 参数的函数
// 参数说明     *ptr                回调参数 用户自拟定的参数指针 不需要的话就传入 NULL
// 返回参数     void
// 使用示例     exti_init(pin, trigger, callback, ptrs);
// 备注信息     以 Line 为准 例如初始化了 A0 就不能初始化 B0 否则就会导致两个 IO 的中断冲突
//-------------------------------------------------------------------------------------------------------------------
void exti_init(gpio_pin_enum pin, exti_trigger_enum trigger, void_callback_uint32_ptr callback, void *ptr)
{
    uint8 io_group = ((pin >> GPIO_GROUP_INDEX_OFFSET) & GPIO_GROUP_INDEX_MASK);
    uint8 io_pin = ((pin >> GPIO_PIN_INDEX_OFFSET) & GPIO_PIN_INDEX_MASK);
    uint32 register_temp = 0;

    gpio_init(pin, GPI, GPIO_HIGH, GPI_PULL_UP); // 初始化选中的引脚

    exti_callback_ptr_list[io_pin] = ptr;
    exti_callback_list[io_pin] = (NULL == callback) ? (exti_callbakc_defalut) : (callback);

    if (15 >= io_pin)
    {
        register_temp = gpio_group[io_group]->POLARITY15_0;
        register_temp &= ~(0x00000003 << ((io_pin % 16) * 2));
        register_temp |= ((uint32)trigger << ((io_pin % 16) * 2));
        gpio_group[io_group]->POLARITY15_0 = register_temp;
    }
    else
    {
        register_temp = gpio_group[io_group]->POLARITY31_16;
        register_temp &= ~(0x00000003 << ((io_pin % 16) * 2));
        register_temp |= ((uint32)trigger << ((io_pin % 16) * 2));
        gpio_group[io_group]->POLARITY31_16 = register_temp;
    }
    gpio_group[io_group]->CPU_INT.ICLR |= (0x00000001 << io_pin);
    gpio_group[io_group]->CPU_INT.IMASK |= (0x00000001 << io_pin);

    interrupt_enable(GPIOA_INT_IRQn);
}
