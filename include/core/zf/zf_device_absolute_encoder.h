/*****************************************

* 文件名称          zf_device_absolute_encoder
* 文件作用          绝对值编码器驱动（SPI 通信、角度读取）

****************************************/

/*****************************************
 * 接线定义：
 *                   ------------------------------------
 *                   模块管脚            单片机管脚
 *                   SCLK                查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_SCLK_PIN 宏定义
 *                   MOSI                查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_MOSI_PIN 宏定义
 *                   MISO                查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_MISO_PIN 宏定义
 *                   CS                  查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_CS_PIN 宏定义
 *                   VCC                 3.3V电源
 *                   GND                 电源地
 *                   ------------------------------------
 ****************************************/

#ifndef _zf_device_absolute_encoder_h_
#define _zf_device_absolute_encoder_h_

#include "zf_common_typedef.h"

//====================================================硬件 SPI 驱动====================================================
#define ABSOLUTE_ENCODER_SPI_SPEED (8 * 1000 * 1000) // 硬件 SPI 速率
#define ABSOLUTE_ENCODER_SPI (SPI_1)                 // 硬件 SPI 号
#define ABSOLUTE_ENCODER_SCLK_PIN (SPI1_SCK_B23)     // 硬件 SPI SCK 引脚
#define ABSOLUTE_ENCODER_MOSI_PIN (SPI1_MOSI_B22)    // 硬件 SPI MOSI 引脚
#define ABSOLUTE_ENCODER_MISO_PIN (SPI1_MISO_B21)    // 硬件 SPI MISO 引脚
//====================================================硬件 SPI 驱动====================================================

#define ABSOLUTE_ENCODER_CS_PIN_MAX (4)
#define ABSOLUTE_ENCODER_CS_PIN_LIST {B17, B18, B26, B27}
#define ABSOLUTE_ENCODER_CS(index, state) ((state) ? (gpio_high(absolute_encoder_cs_list[index])) : (gpio_low(absolute_encoder_cs_list[index])))

#define ABSOLUTE_ENCODER_TIMEOUT_COUNT (100)
#define ABSOLUTE_ENCODER_DEFAULT_ZERO (0)

//====================================================角度传感器参数====================================================
#define ABSOLUTE_ENCODER_SPI_W (0x80)
#define ABSOLUTE_ENCODER_SPI_R (0x40)

#define ABSOLUTE_ENCODER_ZERO_L_REG (0x00)
#define ABSOLUTE_ENCODER_ZERO_H_REG (0x01)
#define ABSOLUTE_ENCODER_DIR_REG (0X09)
//====================================================角度传感器参数====================================================

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器获取当前角度值
// 参数说明     index           编码器索引
// 返回参数     int16           角度值
// 使用示例     absolute_encoder_get_location();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
int16 absolute_encoder_get_location(uint8 index);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器获取相较上次位置的偏移值
// 参数说明     index           编码器索引
// 返回参数     int16           偏移值
// 使用示例     absolute_encoder_get_offset();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
int16 absolute_encoder_get_offset(uint8 index);

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器初始化
// 参数说明     index           编码器索引
// 返回参数     uint8           初始化状态 0-成功 1-失败
// 使用示例     absolute_encoder_init(index);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 absolute_encoder_init(uint8 index);

#endif
