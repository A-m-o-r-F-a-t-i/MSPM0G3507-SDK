/*****************************************

* 文件名称          zf_device_types

****************************************/

#include "zf_device_type.h"

static void type_default_callback(void);

camera_type_enum camera_type = NO_CAMERE;                       // 摄像头类型变量
callback_function camera_uart_handler = type_default_callback;  // 串口通讯中断函数指针，根据初始化时设置的函数进行跳转
callback_function camera_vsync_handler = type_default_callback; // 场中断函数指针，根据初始化时设置的函数进行跳转
callback_function camera_dma_handler = type_default_callback;   // DMA完成中断函数指针，根据初始化时设置的函数进行跳转

wireless_type_enum wireless_type = NO_WIRELESS;
callback_function wireless_module_uart_handler = type_default_callback; // 无线串口接收中断函数指针，根据初始化时设置的函数进行跳转
callback_function wireless_module_spi_handler = type_default_callback;  // WIFI SPI GPIO中断函数指针，根据初始化时设置的函数进行跳转

tof_type_enum tof_type = NO_TOF;
callback_function tof_module_exti_handler = type_default_callback; // ToF 模块 INT 更新中断

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     默认回调函数
// 参数说明     void
// 返回参数     void
// 使用示例
// 备注信息     保护性冗余设计 防止在没有初始化设备的时候跑飞
//-------------------------------------------------------------------------------------------------------------------
static void type_default_callback(void)
{
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     设置摄像头类型
// 参数说明     type_set        选定的摄像头类型
// 参数说明     vsync_callback  设备的场中断回调函数
// 参数说明     dma_callback    设备的 DMA 完成中断回调函数
// 参数说明     uart_callback   设备的串口回调函数
// 返回参数     void
// 使用示例     set_camera_type(CAMERA_GRAYSCALE);
// 备注信息     一般由各摄像头初始化内部调用
//-------------------------------------------------------------------------------------------------------------------
void set_camera_type(camera_type_enum type_set, callback_function vsync_callback, callback_function dma_callback, callback_function uart_callback)
{
    camera_type = type_set;
    camera_uart_handler = ((uart_callback == NULL) ? (type_default_callback) : (uart_callback));
    camera_vsync_handler = ((vsync_callback == NULL) ? (type_default_callback) : (vsync_callback));
    camera_dma_handler = ((dma_callback == NULL) ? (type_default_callback) : (dma_callback));
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     设置无线模块类型
// 参数说明     type_set        选定的无线模块类型
// 参数说明     uart_callback   设备的串口回调函数
// 返回参数     void
// 使用示例     set_wireless_type(WIRELESS_UART, uart_callback);
// 备注信息     一般由各摄像头初始化内部调用
//-------------------------------------------------------------------------------------------------------------------
void set_wireless_type(wireless_type_enum type_set, callback_function wireless_callback)
{
    wireless_type = type_set;
    if (WIFI_SPI == wireless_type)
    {
        wireless_module_spi_handler = ((wireless_callback == NULL) ? (type_default_callback) : (wireless_callback));
    }
    else
    {
        wireless_module_uart_handler = ((wireless_callback == NULL) ? (type_default_callback) : (wireless_callback));
    }
}

//-------------------------------------------------------------------------------------------------------------------
// 函数简介     设置 ToF 模块类型
// 参数说明     type_set        选定的 ToF 模块类型
// 参数说明     exti_callback   设备的外部中断回调函数
// 返回参数     void
// 使用示例     set_tof_type(TOF_DL1A, dl1a_int_handler);
// 备注信息     一般由各摄像头初始化内部调用
//-------------------------------------------------------------------------------------------------------------------
void set_tof_type(tof_type_enum type_set, callback_function exti_callback)
{
    tof_type = type_set;
    tof_module_exti_handler = ((exti_callback == NULL) ? (type_default_callback) : (exti_callback));
}
