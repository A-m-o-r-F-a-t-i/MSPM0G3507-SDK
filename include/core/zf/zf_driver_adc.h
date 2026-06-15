/*****************************************

* 文件名称          zf_driver_adc
* 文件作用          ADC 模数转换驱动（多通道采样、DMA 传输、转换结果读取）

****************************************/

#ifndef _zf_driver_adc_h_
#define _zf_driver_adc_h_

#include "ti_msp_dl_config.h"

#include "zf_common_typedef.h"

#include "zf_driver_gpio.h"

#define ADC_NUM (2)

typedef enum // 枚举 ADC 模块  此枚举定义不允许用户修改
{
    ADC_0,
    ADC_1,
} adc_index_enum;

typedef enum // 枚举 ADC 通道  此枚举定义不允许用户修改
{
    ADC_CHANNEL_0,
    ADC_CHANNEL_1,
    ADC_CHANNEL_2,
    ADC_CHANNEL_3,
    ADC_CHANNEL_4,
    ADC_CHANNEL_5,
    ADC_CHANNEL_6,
    ADC_CHANNEL_7,
} adc_channel_enum;

// 定义引脚要用的宏定义 根据各单片机不同 可以自由定义需要什么信息
// bit[11:0 ] 固定为引脚索引
// bit[15:12] 固定为引脚复用
// bit[19:16] 在 UART 模块中为 UART 索引

#define ADC_PIN_INDEX_OFFSET (0)    // bit[11:0 ] 存储 GPIO 的索引号
#define ADC_PIN_INDEX_MASK (0x0FFF) // 宽度 12bit 因此掩码为 0x0FFF

#define ADC_CHANNEL_OFFSET (12) // bit[15:12] 存储 ADC 通道索引
#define ADC_CHANNEL_MASK (0x0F) // 宽度 4bit 因此掩码为 0x0F

#define ADC_INDEX_OFFSET (16) // bit[19:16] 存储 ADC 索引
#define ADC_INDEX_MASK (0x0F) // 宽度 4bit 因此掩码为 0x0F

typedef enum // 枚举 ADC 引脚通道  此枚举定义不允许用户修改
{
    ADC0_CH0_A27 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_0 << ADC_CHANNEL_OFFSET) | (A27)),
    ADC0_CH1_A26 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_1 << ADC_CHANNEL_OFFSET) | (A26)),
    ADC0_CH2_A25 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_2 << ADC_CHANNEL_OFFSET) | (A25)),
    ADC0_CH3_A24 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_3 << ADC_CHANNEL_OFFSET) | (A24)),
    ADC0_CH4_B25 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_4 << ADC_CHANNEL_OFFSET) | (B25)),
    ADC0_CH5_B24 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_5 << ADC_CHANNEL_OFFSET) | (B24)),
    ADC0_CH6_B20 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_6 << ADC_CHANNEL_OFFSET) | (B20)),
    ADC0_CH7_A22 = ((ADC_0 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_7 << ADC_CHANNEL_OFFSET) | (A22)),

    ADC1_CH0_A15 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_0 << ADC_CHANNEL_OFFSET) | (A15)),
    ADC1_CH1_A16 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_1 << ADC_CHANNEL_OFFSET) | (A16)),
    ADC1_CH2_A17 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_2 << ADC_CHANNEL_OFFSET) | (A17)),
    ADC1_CH3_A18 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_3 << ADC_CHANNEL_OFFSET) | (A18)),
    ADC1_CH4_B17 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_4 << ADC_CHANNEL_OFFSET) | (B17)),
    ADC1_CH5_B18 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_5 << ADC_CHANNEL_OFFSET) | (B18)),
    ADC1_CH6_B19 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_6 << ADC_CHANNEL_OFFSET) | (B19)),
    ADC1_CH7_A21 = ((ADC_1 << ADC_INDEX_OFFSET) | (ADC_CHANNEL_7 << ADC_CHANNEL_OFFSET) | (A21)),
} adc_pin_enum;

typedef enum // 枚举 ADC 分辨率   此枚举定义不允许用户修改
{
    ADC_12BIT, // 12 位分辨率
    ADC_10BIT, // 10 位分辨率
    ADC_8BIT,  // 8  位分辨率
} adc_resolution_enum;

uint16 adc_convert(adc_pin_enum adc_pin);
uint16 adc_mean_filter_convert(adc_pin_enum adc_pin, const uint8 count);
void adc_init(adc_pin_enum adc_pin, adc_resolution_enum resolution);

#endif
