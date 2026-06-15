/*****************************************

* 文件名称          zf_device_absolute_encoder

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

#include "zf_common_clock.h"
#include "zf_common_debug.h"
#include "zf_common_function.h"

#include "zf_driver_delay.h"
#include "zf_driver_spi.h"

#include "zf_device_absolute_encoder.h"

static int16 now_location[4] = {0};
static int16 last_location[4] = {0};

gpio_pin_enum absolute_encoder_cs_list[ABSOLUTE_ENCODER_CS_PIN_MAX] = ABSOLUTE_ENCODER_CS_PIN_LIST;

#define absolute_encoder_read() (spi_read_8bit(ABSOLUTE_ENCODER_SPI))
#define absolute_encoder_write(data) (spi_write_8bit(ABSOLUTE_ENCODER_SPI, (data)))

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器写寄存器
// 参数说明     index           编码器索引
// 参数说明     reg             寄存器地址
// 参数说明     data            数据
// 返回参数     void
// 使用示例     absolute_encoder_write_register(i + 1, dat[i]);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static void absolute_encoder_write_register(uint8 index, uint8 reg, uint8 data)
{
    ABSOLUTE_ENCODER_CS(index, 0);                        // 片选拉低选中
    absolute_encoder_write(reg | ABSOLUTE_ENCODER_SPI_W); // 寄存器
    absolute_encoder_write(data);                         // 数据
    ABSOLUTE_ENCODER_CS(index, 1);                        // 片选拉高释放
    system_delay_us(1);                                   // 必要操作
    ABSOLUTE_ENCODER_CS(index, 0);                        // 片选拉低选中
    absolute_encoder_read();                              // 这里会返回写入是否成功 但不作判断
    absolute_encoder_read();                              // 必要操作
    ABSOLUTE_ENCODER_CS(index, 1);                        // 片选拉高释放
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器读寄存器 内部调用
// 参数说明     index           编码器索引
// 参数说明     reg             寄存器地址
// 返回参数     uint8           数据
// 使用示例     absolute_encoder_read_register(6);
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static uint8 absolute_encoder_read_register(uint8 index, uint8 reg)
{
    uint8 data = 0;
    ABSOLUTE_ENCODER_CS(index, 0);                        // 片选拉低选中
    absolute_encoder_write(reg | ABSOLUTE_ENCODER_SPI_R); // 寄存器
    absolute_encoder_write(0x00);                         // 占位
    ABSOLUTE_ENCODER_CS(index, 1);                        // 片选拉高释放
    system_delay_us(1);                                   // 必要操作
    ABSOLUTE_ENCODER_CS(index, 0);                        // 片选拉低选中
    data = absolute_encoder_read();                       // 获取读取的数据
    absolute_encoder_read();                              // 必要操作
    ABSOLUTE_ENCODER_CS(index, 1);                        // 片选拉高释放
    return data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器读位置 内部调用
// 参数说明     index           编码器索引
// 返回参数     uint16          位置值
// 使用示例     absolute_encoder_read_data();
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static uint16 absolute_encoder_read_data(uint8 index)
{
    uint16 data = 0;
    ABSOLUTE_ENCODER_CS(index, 0);   // 片选拉低选中
    data = absolute_encoder_read();  // 获取高八位数据
    data = (data & 0x00FF) << 8;     // 数据位移
    data |= absolute_encoder_read(); // 获取低八位数据
    ABSOLUTE_ENCODER_CS(index, 1);   // 片选拉高释放
    return data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器自检 内部调用
// 参数说明     index           编码器索引
// 返回参数     uint8           自检状态
// 使用示例     absolute_encoder_self_check();
// 备注信息     内部调用
//-------------------------------------------------------------------------------------------------------------------
static uint8 absolute_encoder_self_check(uint8 index)
{
    uint8 i = 0, return_state = 0;
    uint8 dat[6] = {0, 0, 0, 0xC0, 0xFF, 0x1C};
    uint16 time_count = 0;
    while (0x1C != absolute_encoder_read_register(index, 6)) // 获取状态寄存器
    {
        for (i = 0; 6 > i; i++)
        {
            absolute_encoder_write_register(index, i + 1, dat[i]); // 写入默认配置参数
            system_delay_ms(1);
        }
        if (ABSOLUTE_ENCODER_TIMEOUT_COUNT < time_count++) // 等待超时
        {
            return_state = 1;
            break;
        }
    }
    return return_state;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器获取当前角度值
// 参数说明     index           编码器索引
// 返回参数     int16           角度值
// 使用示例     absolute_encoder_get_location();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
int16 absolute_encoder_get_location(uint8 index)
{
    last_location[index] = now_location[index];
    now_location[index] = absolute_encoder_read_data(index) >> 4;
    return now_location[index];
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器获取相较上次位置的偏移值
// 参数说明     index           编码器索引
// 返回参数     int16           偏移值
// 使用示例     absolute_encoder_get_offset();
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
int16 absolute_encoder_get_offset(uint8 index)
{
    int16 result_data = 0;
    if (2048 < func_abs(now_location[index] - last_location[index]))
    {
        result_data = (2048 < now_location[index] ? (now_location[index] - 4096 - last_location[index]) : (now_location[index] + 4096 - last_location[index]));
    }
    else
    {
        result_data = (now_location[index] - last_location[index]);
    }
    return result_data;
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     绝对值编码器初始化
// 参数说明     index           编码器索引
// 返回参数     uint8           初始化状态 0-成功 1-失败
// 使用示例     absolute_encoder_init(index);
// 备注信息
//-------------------------------------------------------------------------------------------------------------------
uint8 absolute_encoder_init(uint8 index)
{
    uint8 return_state = 0;
    uint16 zero_position = ABSOLUTE_ENCODER_DEFAULT_ZERO;

    spi_init(ABSOLUTE_ENCODER_SPI, SPI_MODE0, ABSOLUTE_ENCODER_SPI_SPEED, ABSOLUTE_ENCODER_SCLK_PIN, ABSOLUTE_ENCODER_MOSI_PIN, ABSOLUTE_ENCODER_MISO_PIN, SPI_CS_NULL);

    gpio_init(absolute_encoder_cs_list[index], GPO, GPIO_LOW, GPO_PUSH_PULL);

    do
    {
        if (absolute_encoder_self_check(index))
        {
            // 如果程序在输出了断言信息 并且提示出错位置在这里
            // 那么就是绝对值编码器自检出错并超时退出了
            // 检查一下接线有没有问题 如果没问题可能就是坏了
            return_state = 1;
            zf_log(0, "absolute encoder init errror.");
            break;
        }
        absolute_encoder_write_register(index, ABSOLUTE_ENCODER_DIR_REG, 0x00); // 设置旋转方向 正转数值变小：0x00   反转数值变大：0x80
        zero_position = (uint16)(4096 - zero_position);
        zero_position = zero_position << 4;
        absolute_encoder_write_register(index, ABSOLUTE_ENCODER_ZERO_L_REG, (uint8)zero_position); // 设置零位
        absolute_encoder_write_register(index, ABSOLUTE_ENCODER_ZERO_H_REG, zero_position >> 8);
    } while (0);
    return return_state;
}
