/*********************************************************************************************************************
* MSPM0G3507 Opensource Library 即（MSPM0G3507 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
* 
* 本文件是 MSPM0G3507 开源库的一部分
* 
* MSPM0G3507 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
* 
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
* 
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
* 
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
* 
* 文件名称          mian
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          MDK 5.37
* 适用平台          MSPM0G3507
* 店铺链接          https://seekfree.taobao.com/
* 
* 修改记录
* 日期              作者                备注
* 2025-06-1        SeekFree            first version
********************************************************************************************************************/

#include "zf_common_headfile.h"
// 打开新的工程或者工程移动了位置务必执行以下操作
// 第一步 关闭上面所有打开的文件
// 第二步 project->clean  等待下方进度条走完


// *************************** 例程硬件连接说明 ***************************
// 使用逐飞科技 CMSIS-DAP | ARM 调试下载器连接
//      直接将下载器正确连接在核心板的调试下载接口即可
// 
// 使用 USB-TTL 模块连接
//      模块管脚            单片机管脚
//      USB-TTL-RX          查看 zf_common_debug.h 文件中 DEBUG_UART_TX_PIN 宏定义的引脚 默认 A10
//      USB-TTL-TX          查看 zf_common_debug.h 文件中 DEBUG_UART_RX_PIN 宏定义的引脚 默认 A11
//      USB-TTL-GND         核心板电源地 GND
//      USB-TTL-3V3         核心板 3V3 电源
// 
// 绝对值编码器（角度传感器）接线
//      模块管脚            单片机管脚
//      SCLK                查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_SCLK_PIN 宏定义
//      MOSI                查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_MOSI_PIN 宏定义
//      MISO                查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_MISO_PIN 宏定义
//      CS                  查看 zf_device_absolute_encoder.h 中 ABSOLUTE_ENCODER_CS_PIN 宏定义 4个编码器分别使用不同的CS
//      GND                 核心板电源地 GND
//      3V3                 核心板 3V3 电源



// *************************** 例程测试说明 ***************************
// 1.核心板烧录本例程 插在主板上 绝对值编码器按照上述硬件连接方式接好
// 
// 2.电池供电 上电后会从串口输出编码器采集的位置信息 如下
//      location =  xxxx,xxxx,xxxx,xxxx
//      offset =    xxxx,xxxx,xxxx,xxxx
// 
// 3.转动编码器数据会有变化
// 
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************

// 学习板上最多支持接入四个绝对值编码器
#define ENCODER_INEX_MAX                ( 4 )

// 按照实际转速选择合适的采样周期 转速越快那么要求采样周期越短
// 如果转速较高而采样周期过长 会导致一个采样周期内编码器转动超过一整圈 从而导致数据丢失
// 因此需要选择合适的采样周期 默认 5ms 周期符合大多数使用场景
#define ENCODER_SAMPLE_PERIOD_MS        ( 5 )

volatile uint8_t pit_flag    = 0;

uint8 encoder_index = 0;
int16 location_data[ENCODER_INEX_MAX] = {0, 0, 0, 0};
int16 offset_data[ENCODER_INEX_MAX]   = {0, 0, 0, 0};

void absolute_encoder_pit_handler (uint32 event, void *ptr)
{
    *((uint8 *)ptr)  = 1;

    for(encoder_index = 0; encoder_index < ENCODER_INEX_MAX; encoder_index ++)
    {
        // 获取编码器当前的角度信息
        location_data[encoder_index] = absolute_encoder_get_location(encoder_index);
        // 通过两次角度对比得到当前的旋转速度
        offset_data[encoder_index] = absolute_encoder_get_offset(encoder_index);
    }
}

int main (void)
{
    clock_init(SYSTEM_CLOCK_80M);                                               // 时钟配置及系统初始化<务必保留>
    debug_init();                                                               // 调试串口信息初始化
    // 此处编写用户代码 例如外设初始化代码等

    for(encoder_index = 0; encoder_index < ENCODER_INEX_MAX; encoder_index ++)
    {
        if(absolute_encoder_init(encoder_index))                                //初始化4个编码器
        {

            printf("encoder %d fail\r\n", encoder_index + 1);                   //提示编码器初始化失败
        }
        else
        {
            printf("encoder %d successfully\r\n", encoder_index + 1);           //提示编码器初始化成功
        }
        system_delay_ms(500);
    }

    pit_ms_init(PIT_TIM_G12, ENCODER_SAMPLE_PERIOD_MS, absolute_encoder_pit_handler, (void *)&pit_flag);

    while(true)
    {
        // 此处编写需要循环执行的代码
        if(pit_flag)                                                            //在串口上位机上显示角度和转速信息
        {
            printf("location =  %d, %d, %d, %d\r\n",
                location_data[0], location_data[1], location_data[2], location_data[3]);
            printf("offset   =  %d, %d, %d, %d\r\n",
                offset_data[0], offset_data[1], offset_data[2], offset_data[3]);
            pit_flag = 0;
            system_delay_ms(100);
        }
        // 此处编写需要循环执行的代码
    }
}

// ------------------------------ 关于编码器型号类型的简易说明 ------------------------------
// 本开源库中使用的是 SPI 接口的 360°位置传感器 绝对值编码器

// 这是由于 MSPM0 系列 仅有 TIMG8/9/10/11 支持 QEI/HALL Input Mode
// QEI/HALL Input Mode 才能直接无额外开销支持接入增量式编码器 即 正交编码器 和 方向编码器
// 而 MSPM0G3507 仅有一个 TIMG8 也就只能无额外开销接入一个 正交编码器 或 方向编码器

// 如果一定要使用多个 正交编码器 或 方向编码器 那么只能通过 外部中断 脉宽捕获 的方式
// 再额外加上换算开销 才能完成编码器的数据采集
// 这种方式导致需要频繁触发 3507 的外部中断 会导致以下问题
// 问题一 当编码器线数较高（超过256）时 可能会导致容易触发 3507 的保护从而锁定芯片
// 问题二 它会每个脉冲或者每个脉冲周期都进行中断与计算 线数越高开销越大

// 因此我们建议使用 SPI 接口的 360°位置传感器 绝对值编码器
// 避免上述两个问题 减少中断与开销
// 当然自身技术能力较强的同学 可以选择自行解决这个问题 使用多个 正交编码器 或 方向编码器
// ------------------------------ 关于编码器型号类型的简易说明 ------------------------------
