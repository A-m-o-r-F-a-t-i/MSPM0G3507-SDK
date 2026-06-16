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

// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中
// *************************** 例程硬件连接说明 ***************************
// 接入无线转串口模块
//      模块管脚            单片机管脚
//      RX                  查看 zf_device_wrieless_uart.h 中 WIRELESS_UART_RX_PINx  宏定义
//      TX                  查看 zf_device_wrieless_uart.h 中 WIRELESS_UART_TX_PINx  宏定义
//      RTS                 查看 zf_device_wrieless_uart.h 中 WIRELESS_UART_RTS_PINx 宏定义
//      GND                 核心板电源地 GND
//      3V3                 核心板 3V3 电源

// **************************** 代码区域 ****************************
uint8 data_buffer[32];
uint8 data_len;


int main(void)
{
    clock_init(SYSTEM_CLOCK_80M); 	// 时钟配置及系统初始化<务必保留>
    debug_init();                   // 调试串口信息初始化
    
    // 此处编写用户代码 例如外设初始化代码等
    uint8 count = 0;

    if(wireless_uart_init())                                                    // 判断是否通过初始化
    {
        while(1)                                                                // 初始化失败就在这进入死循环
        {
			printf("wireless_uart_init Fail !!!\r\n");
            system_delay_ms(100);                                               // 初始化异常
        }
    }
		
    wireless_uart_send_byte('\r');
    wireless_uart_send_byte('\n');
    wireless_uart_send_string("SEEKFREE wireless uart demo.\r\n");              // 初始化正常 输出测试信息

    // 此处编写用户代码 例如外设初始化代码等
    while(true)
    {
        // 此处编写需要循环执行的代码

        data_len = (uint8)wireless_uart_read_buffer(data_buffer, 32);         	// 查看是否有消息 默认缓冲区是 WIRELESS_UART_BUFFER_SIZE 总共 64 字节
        if(data_len != 0)                                                       // 收到了消息 读取函数会返回实际读取到的数据个数
        {
            wireless_uart_send_buffer(data_buffer, data_len);                	// 将收到的消息发送回去
            memset(data_buffer, 0, 32);
            func_uint_to_str((char *)data_buffer, data_len);
            wireless_uart_send_string("\r\ndata len:");                              	// 显示实际收到的数据信息
            wireless_uart_send_buffer(data_buffer, strlen((const char *)data_buffer));	// 显示收到的数据个数
            wireless_uart_send_string(".\r\n");
        }
        if(count ++ > 10)
        {
            count = 0;
        }
        system_delay_ms(50);

        // 此处编写需要循环执行的代码
    }
}

// **************************** 代码区域 ****************************
// *************************** 例程常见问题说明 ***************************
// 遇到问题时请按照以下问题检查列表检查
// 问题1：串口没有数据
//      查看 逐飞助手 打开的是否是正确的串口，检查打开的 COM 口是否对应的是 无线转串口模块 的 COM 口
//      查看核心板LED灯状态 如果快速闪烁证明初始化失败 检查接线 或者按照说明书检查模块是否正常
// 问题2：串口数据乱码
//      查看是否模块波特率被修改过 恢复出厂设置
// 问题3：发送数据没有收到回复
//      调试看看是否读取到数据 data_len 是否一直都是 如果是 那么按照说明书检查模块是否正常