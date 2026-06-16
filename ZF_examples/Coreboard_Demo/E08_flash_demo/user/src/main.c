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
// 使用逐飞科技 CMSIS-DAP 调试下载器连接
//      直接将下载器正确连接在核心板的调试下载接口即可
// 使用 USB-TTL 模块连接
//      模块管脚            单片机管脚
//      USB-TTL-RX          查看 zf_common_debug.h 文件中 DEBUG_UART_TX_PIN 宏定义的引脚 默认 P14_0
//      USB-TTL-TX          查看 zf_common_debug.h 文件中 DEBUG_UART_RX_PIN 宏定义的引脚 默认 P14_1
//      USB-TTL-GND         核心板电源地 GND
//      USB-TTL-3V3         核心板 3V3 电源


// *************************** 例程测试说明 ***************************
// 1.核心板烧录完成本例程，单独使用核心板与调试下载器或者 USB-TTL 模块，在断电情况下完成连接
// 2.将调试下载器或者 USB-TTL 模块连接电脑，完成上电
// 3.电脑上使用串口助手打开对应的串口，串口波特率为 zf_common_debug.h 文件中 DEBUG_UART_BAUDRATE 宏定义 默认 115200，核心板按下复位按键
// 4.可以在串口助手上看到如下串口信息：
//      FLASH_SECTION_INDEX: 0, FLASH_PAGE_INDEX: 0, origin data is :
//      ...
// 如果发现现象与说明严重不符 请参照本文件最下方 例程常见问题说明 进行排查

// **************************** 代码区域 ****************************

#define FLASH_SECTION_INDEX       (0)                                 // 存储数据用的扇区
#define FLASH_PAGE_INDEX          (0)                                 // 存储数据用的页码 倒数第一个页码

int main(void)
{
    clock_init(SYSTEM_CLOCK_80M); 	// 时钟配置及系统初始化<务必保留>
    debug_init();                   // 调试串口信息初始化
    // 此处编写用户代码 例如外设初始化代码等
																				// 使用flash前先调用flash初始化
    
    if(flash_check(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX))                      // 判断是否有数据
        flash_erase_page(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);                // 擦除这一页


    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);        // 将数据从 flash 读取到缓冲区
    printf("\r\n FLASH_SECTION_INDEX: %d, FLASH_PAGE_INDEX: %d, origin data is :", FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以 float  格式输出
    printf("\r\n uint32_type: %u", flash_union_buffer[1].uint32_type);          // 将缓冲区第 1 个位置的数据以 uint32 格式输出
    printf("\r\n int32_type : %d", flash_union_buffer[2].int32_type);           // 将缓冲区第 2 个位置的数据以 int32  格式输出
    printf("\r\n uint16_type: %u", flash_union_buffer[3].uint16_type);          // 将缓冲区第 3 个位置的数据以 uint16 格式输出
    printf("\r\n int16_type : %d", flash_union_buffer[4].int16_type);           // 将缓冲区第 4 个位置的数据以 int16  格式输出
    printf("\r\n uint8_type : %u", flash_union_buffer[5].uint8_type);           // 将缓冲区第 5 个位置的数据以 uint8  格式输出
    printf("\r\n int8_type  : %d", flash_union_buffer[6].int8_type);            // 将缓冲区第 6 个位置的数据以 int8   格式输出
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据

    // 例如 flash_data_union_buffer[0] 写入 int8_type 那么只能以 int8_type 读取
    // 同样 flash_data_union_buffer[0] 写入 float_type 那么只能以 float_type 读取
    printf("\r\n");
    printf("\r\n Data property display :");
    printf("\r\n flash_data_union_buffer[0] write int8 data type:");
    flash_union_buffer[0].int8_type   = -128;                                   // 向缓冲区第 0 个位置写入     int8   数据
    printf("\r\n float_type : %f-data error", flash_union_buffer[0].float_type);// 将缓冲区第 0 个位置的数据以  float  格式输出 数据将不正确
    printf("\r\n int8_type  : %d", flash_union_buffer[0].int8_type);            // 将缓冲区第 0 个位置的数据以  int8   格式输出 得到正确写入数据

    printf("\r\n flash_data_union_buffer[0] write int8 data type:");
    flash_union_buffer[0].int8_type  -= 1;                                      // 向缓冲区第 0 个位置写入     int8   数据
    printf("\r\n float_type : %f-data error", flash_union_buffer[0].float_type);// 将缓冲区第 0 个位置的数据以  float  格式输出 数据将不正确
    printf("\r\n int8_type  : %d", flash_union_buffer[0].int8_type);            // 将缓冲区第 0 个位置的数据以  int8   格式输出 得到正确写入数据

    printf("\r\n flash_data_union_buffer[0] write float data type:");
    flash_union_buffer[0].float_type  = 16.625;                                 // 向缓冲区第 0 个位置写入     float  数据
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以  float  格式输出 得到正确写入数据
    printf("\r\n int8_type  : %d-data error", flash_union_buffer[0].int8_type); // 将缓冲区第 0 个位置的数据以  int8   格式输出 数据将不正确

    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据
    // 请注意 数据缓冲区的每个位置只能存储一种类型的数据

    flash_buffer_clear();                                                       // 清空缓冲区
    flash_union_buffer[0].float_type  = 3.1415926;                              // 向缓冲区第 0 个位置写入 float  数据
    flash_union_buffer[1].uint32_type = 4294967295;                             // 向缓冲区第 1 个位置写入 uint32 数据
    flash_union_buffer[2].int32_type  = -21474833;                              // 向缓冲区第 2 个位置写入 int32  数据
    flash_union_buffer[3].uint16_type = 65535;                                  // 向缓冲区第 3 个位置写入 uint16 数据
    flash_union_buffer[4].int16_type  = -32768;                                 // 向缓冲区第 4 个位置写入 int16  数据
    flash_union_buffer[5].uint8_type  = 255;                                    // 向缓冲区第 5 个位置写入 uint8  数据
    flash_union_buffer[6].int8_type   = -128;                                   // 向缓冲区第 6 个位置写入 int8   数据
    flash_write_page_from_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);     // 向指定 Flash 扇区的页码写入缓冲区数据

    printf("\r\n");
    flash_buffer_clear();                                                       // 清空缓冲区
    printf("\r\n Flash data buffer default data is :");
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以 float  格式输出
    printf("\r\n uint32_type: %u", flash_union_buffer[1].uint32_type);          // 将缓冲区第 1 个位置的数据以 uint32 格式输出
    printf("\r\n int32_type : %d", flash_union_buffer[2].int32_type);           // 将缓冲区第 2 个位置的数据以 int32  格式输出
    printf("\r\n uint16_type: %u", flash_union_buffer[3].uint16_type);          // 将缓冲区第 3 个位置的数据以 uint16 格式输出
    printf("\r\n int16_type : %d", flash_union_buffer[4].int16_type);           // 将缓冲区第 4 个位置的数据以 int16  格式输出
    printf("\r\n uint8_type : %u", flash_union_buffer[5].uint8_type);           // 将缓冲区第 5 个位置的数据以 uint8  格式输出
    printf("\r\n int8_type  : %d", flash_union_buffer[6].int8_type);            // 将缓冲区第 6 个位置的数据以 int8   格式输出
    system_delay_ms(200);
    printf("\r\n");
    flash_read_page_to_buffer(FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);        // 将数据从 flash 读取到缓冲区
    printf("\r\n FLASH_SECTION_INDEX: %d, FLASH_PAGE_INDEX: %d, new data is :", FLASH_SECTION_INDEX, FLASH_PAGE_INDEX);
    printf("\r\n float_type : %f", flash_union_buffer[0].float_type);           // 将缓冲区第 0 个位置的数据以 float  格式输出
    printf("\r\n uint32_type: %u", flash_union_buffer[1].uint32_type);          // 将缓冲区第 1 个位置的数据以 uint32 格式输出
    printf("\r\n int32_type : %d", flash_union_buffer[2].int32_type);           // 将缓冲区第 2 个位置的数据以 int32  格式输出
    printf("\r\n uint16_type: %u", flash_union_buffer[3].uint16_type);          // 将缓冲区第 3 个位置的数据以 uint16 格式输出
    printf("\r\n int16_type : %d", flash_union_buffer[4].int16_type);           // 将缓冲区第 4 个位置的数据以 int16  格式输出
    printf("\r\n uint8_type : %u", flash_union_buffer[5].uint8_type);           // 将缓冲区第 5 个位置的数据以 uint8  格式输出
    printf("\r\n int8_type  : %d", flash_union_buffer[6].int8_type);            // 将缓冲区第 6 个位置的数据以 int8   格式输出
    
    
    // 此处编写用户代码 例如外设初始化代码等
    while(true)
    {
        // 此处编写需要循环执行的代码


      
        
        // 此处编写需要循环执行的代码
    }
}