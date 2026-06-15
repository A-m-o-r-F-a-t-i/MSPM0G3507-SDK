/*****************************************

* 文件名称          zf_driver_adc

****************************************/

#include "zf_common_clock.h"

#include "zf_driver_adc.h"

static ADC12_Regs *adc_list[2] = {ADC0, ADC1};           // 模块索引数组
static uint8 adc_resolution[2] = {ADC_12BIT, ADC_12BIT}; // 精度数据备份

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     adc 转换数据
// 参数说明     adc_pin         选择 adc 通道 (详见 zf_driver_adc.h 中枚举 adc_pin_enum 定义)
// 返回参数     uint16          转换的 adc 值
// 使用示例     adc_convert(ADC1_CH0_A0);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 adc_convert(adc_pin_enum adc_pin)
{
    uint8 adc_index = (adc_pin >> ADC_INDEX_OFFSET) & ADC_INDEX_MASK;
    uint8 adc_channel = (adc_pin >> ADC_CHANNEL_OFFSET) & ADC_CHANNEL_MASK;
    uint16 return_value = 0;

    DL_ADC12_disableConversions(adc_list[adc_index]);
    DL_ADC12_configConversionMem(
        adc_list[adc_index], DL_ADC12_MEM_IDX_0,
        adc_channel, DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    DL_ADC12_enableConversions(adc_list[adc_index]);

    DL_ADC12_startConversion(adc_list[adc_index]);
    while (!(adc_list[adc_index]->ULLMEM.CPU_INT.RIS & ADC12_CPU_INT_RIS_MEMRESIFG0_SET))
        ;
    DL_ADC12_stopConversion(adc_list[adc_index]);
    return_value = DL_ADC12_getMemResult(adc_list[adc_index], DL_ADC12_MEM_IDX_0);

    return return_value;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     adc 均值滤波转换
// 参数说明     adc_pin         选择 adc 通道 (详见 zf_driver_adc.h 中枚举 adc_pin_enum 定义)
// 参数说明     count           均值滤波次数
// 返回参数     uint16          转换的 adc 值
// 使用示例     adc_mean_filter_convert(ADC1_CH0_A0, 5);                        // 采集5次 然后返回平均值
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint16 adc_mean_filter_convert(adc_pin_enum adc_pin, const uint8 count)
{
    uint8 i = 0;
    uint32 sum = 0;

    for (i = 0; i < count; i++)
    {
        sum += adc_convert(adc_pin);
    }

    return sum / count;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     adc 初始化
// 参数说明     adc_pin         选择 adc 通道 (详见 zf_driver_adc.h 中枚举 adc_pin_enum 定义)
// 参数说明     resolution      选择选择通道分辨率(如果同一个 adc 模块初始化时设置了不同的分辨率 则最后一个初始化的分辨率生效)
// 返回参数     void
// 使用示例     adc_init(ADC1_CH0_A0, ADC_8BIT);                                // 初始化 A0 为 ADC1 的 channel0 输入功能 分辨率为8位
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
void adc_init(adc_pin_enum adc_pin, adc_resolution_enum resolution)
{
    static const DL_ADC12_ClockConfig adc_clock_config =
        {
            .clockSel = DL_ADC12_CLOCK_HFCLK,
            .divideRatio = DL_ADC12_CLOCK_DIVIDE_1,
            .freqRange = DL_ADC12_CLOCK_FREQ_RANGE_32_TO_40,
        };

    uint8 adc_index = (adc_pin >> ADC_INDEX_OFFSET) & ADC_INDEX_MASK;
    uint8 adc_channel = (adc_pin >> ADC_CHANNEL_OFFSET) & ADC_CHANNEL_MASK;

    gpio_init((gpio_pin_enum)(adc_pin >> ADC_PIN_INDEX_OFFSET) & ADC_PIN_INDEX_MASK, GPI, GPIO_LOW, GPI_ANAOG_IN);

    DL_ADC12_disableConversions(adc_list[adc_index]);
    DL_ADC12_setClockConfig(adc_list[adc_index], (DL_ADC12_ClockConfig *)&adc_clock_config);

    DL_ADC12_setStartAddress(adc_list[adc_index], DL_ADC12_SEQ_START_ADDR_00);
    DL_ADC12_configConversionMem(
        adc_list[adc_index], DL_ADC12_MEM_IDX_0,
        adc_channel, DL_ADC12_REFERENCE_VOLTAGE_VDDA,
        DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0, DL_ADC12_AVERAGING_MODE_DISABLED,
        DL_ADC12_BURN_OUT_SOURCE_DISABLED, DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
        DL_ADC12_WINDOWS_COMP_MODE_DISABLED);
    adc_list[adc_index]->ULLMEM.CTL2 &= ~ADC12_CTL2_RES_MASK;
    adc_list[adc_index]->ULLMEM.CTL2 |= (resolution << ADC12_CTL2_RES_OFS);
    adc_list[adc_index]->ULLMEM.CTL0 |= ADC12_CTL0_PWRDN_MANUAL;
    DL_ADC12_enableConversions(adc_list[adc_index]);
}
