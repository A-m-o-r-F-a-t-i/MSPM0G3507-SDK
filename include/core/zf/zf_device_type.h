/*****************************************

* 文件名称          zf_device_type
* 文件作用          设备类型定义（摄像头、无线模块等外设类型枚举）

****************************************/

#ifndef _zf_device_type_h_
#define _zf_device_type_h_

#include "zf_common_typedef.h"

//==============================================定义 外设 参数结构体==================================================
typedef enum
{
    NO_CAMERE = 0,    // 无摄像头
    CAMERA_BIN_IIC,   // 小钻风 IIC 版本
    CAMERA_BIN_UART,  // 小钻风 UART 版本
    CAMERA_GRAYSCALE, // 总钻风
    CAMERA_COLOR,     // 凌瞳
} camera_type_enum;

typedef enum
{
    NO_WIRELESS = 0,  // 无设备
    WIRELESS_UART,    // 无线串口
    BLUETOOTH_CH9141, // 蓝牙 CH9141
    WIFI_UART,        // 串口 WiFi
    WIFI_SPI          // SPI WiFi
} wireless_type_enum;

typedef enum
{
    NO_TOF = 0, // 无设备
    TOF_DL1A,   // DL1A
    TOF_DL1B,   // DL1B
} tof_type_enum;
//==============================================定义 外设 参数结构体==================================================

//===========================================声明 回调函数指针及外设 类型==============================================
typedef void (*callback_function)(void);

extern wireless_type_enum wireless_type;
extern callback_function wireless_module_uart_handler; // 无线串口接收中断函数指针，根据初始化时设置的函数进行跳转
extern callback_function wireless_module_spi_handler;  // WIFI SPI GPIO中断函数指针，根据初始化时设置的函数进行跳转

extern camera_type_enum camera_type;
extern callback_function camera_dma_handler;   // 串口通讯中断函数指针，根据初始化时设置的函数进行跳转
extern callback_function camera_vsync_handler; // 串口通讯中断函数指针，根据初始化时设置的函数进行跳转
extern callback_function camera_uart_handler;  // 串口通讯中断函数指针，根据初始化时设置的函数进行跳转

extern tof_type_enum tof_type;                    // ToF 模块 类型
extern callback_function tof_module_exti_handler; // ToF 模块 INT 更新中断
//===========================================声明 回调函数指针及外设 类型==============================================

//=============================================声明 中断回调 基础函数================================================
void set_camera_type(camera_type_enum type_set, callback_function vsync_callback, callback_function dma_callback, callback_function uart_callback);
void set_wireless_type(wireless_type_enum type_set, callback_function wireless_callback);
void set_tof_type(tof_type_enum type_set, callback_function exti_callback);
//=============================================声明 中断回调 基础函数================================================

#endif
