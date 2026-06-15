/*****************************************

* 文件名称          zf_common_typedef
* 文件作用          公共类型定义 - 定义逐飞库基础数据类型(uint8/uint16/uint32等)、布尔值、使能禁用宏、函数指针类型以及IDE适配宏

****************************************/

#ifndef _zf_common_typedef_h_
#define _zf_common_typedef_h_

// 标准库
#include "stdio.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdarg.h"
#include "string.h"
#include "stdlib.h"

// 注释的标准缩进 空行的 20 个 Tab 键为基础

// 此处定义 常用类型 如果与 SDK 冲突的话就将 USE_ZF_TYPEDEF 设置为 0
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Part Start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define USE_ZF_TYPEDEF (1) // 是否启用类型定义申明

#if USE_ZF_TYPEDEF
// 数据类型声明
// 尽量使用 stdint.h 定义的类型名称 避免冲突 这里可以裁剪
typedef unsigned char uint8;       // 无符号  8 bits
typedef unsigned short int uint16; // 无符号 16 bits
typedef unsigned int uint32;       // 无符号 32 bits
typedef unsigned long long uint64; // 无符号 64 bits

typedef signed char int8;       // 有符号  8 bits
typedef signed short int int16; // 有符号 16 bits
typedef signed int int32;       // 有符号 32 bits
typedef signed long long int64; // 有符号 64 bits

typedef volatile uint8 vuint8;   // 易变性修饰 无符号  8 bits
typedef volatile uint16 vuint16; // 易变性修饰 无符号 16 bits
typedef volatile uint32 vuint32; // 易变性修饰 无符号 32 bits
typedef volatile uint64 vuint64; // 易变性修饰 无符号 64 bits

typedef volatile int8 vint8;   // 易变性修饰 有符号  8 bits
typedef volatile int16 vint16; // 易变性修饰 有符号 16 bits
typedef volatile int32 vint32; // 易变性修饰 有符号 32 bits
typedef volatile int64 vint64; // 易变性修饰 有符号 64 bits
#endif

typedef enum
{
    COMMON_DATA_SIZE_8BIT = 1,  // 数据位宽 8bit
    COMMON_DATA_SIZE_16BIT = 2, // 数据位宽 16bit
    COMMON_DATA_SIZE_32BIT = 4, // 数据位宽 32bit
} common_data_size_enum;

#define ZF_NO_ERROR (0) // 通用状态返回检查值 通常情况下返回 0 代表无异常
#define ZF_ERROR (1)    // 通用状态返回检查值 通常情况下返回 非 0 值代表异常码

#define ZF_ENABLE (1)  // 使能 定义
#define ZF_DISABLE (0) // 禁止 定义

#define ZF_TRUE (1)  // 布尔值 真 定义
#define ZF_FALSE (0) // 布尔值 假 定义
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Part   End <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 此处定义 常用函数指针类型
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Part Start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
typedef void (*void_function_void)(void);
typedef void (*void_function_uint32)(uint32 parameter);
typedef void (*void_function_ptr)(void *ptr);

typedef uint32 (*uint32_function_void)(void);
typedef uint32 (*uint32_function_uint32)(uint32 parameter);
typedef uint32 (*uint32_function_ptr)(void *ptr);

typedef void *(*ptr_function_void)(void);
typedef void *(*ptr_function_uint32)(uint32 parameter);
typedef void *(*ptr_function_ptr)(void *ptr);

typedef void (*void_callback_uint32_ptr)(uint32 state, void *ptr);
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Part   End <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

// 此处定义 IDE 适配 因为不同 IDE 工具链有差异
// >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>> Part Start >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
#define IDE_MDK (0x01)
#define IDE_IAR (0x02)
#define IDE_ADS (0x04)
#define IDE_MRS (0x08)

#if defined(__ICCARM__)
#define IDE_TYPE (IDE_IAR)
#else
#define IDE_TYPE (IDE_MDK)
#endif

#if (IDE_MDK & IDE_TYPE)
// -------------------------- MDK IDE 适配 --------------------------
// MDK 使用 GNUC 工具链
// 内联函数修饰
#define ZF_INLINE static inline
#define ZF_WEAK __attribute__((weak))

// 内存对齐与紧凑结构
#define ZF_PACKED __attribute__((packed))
#define ZF_PACKED_ENABLE
#define ZF_PACKED_DISABLE

// 内存屏障指令 用于同步数据与指令 防止工程开优化导致问题
#define ZF_DSB() __DSB()
#define ZF_ISB() __ISB()
#define ZF_DMB() __DMB()

// 文件定位
#define ZF_FILE_MESSAGE (__FILE__)
#define ZF_LINE_MESSAGE (__LINE__)
// -------------------------- MDK IDE 适配 --------------------------
#elif (IDE_IAR & IDE_TYPE)
// -------------------------- IAR IDE 适配 --------------------------
// IAR 使用 ICCARM 工具链
// 内联函数修饰
#define ZF_INLINE static inline
#define ZF_WEAK __attribute__((weak))

// 内存对齐与紧凑结构
// IAR 支持 __attribute__((packed)) 因此不需要 _Pragma
#define ZF_PACKED __attribute__((packed))
#define ZF_PACKED_ENABLE  // _Pragma("pack(push,1)")
#define ZF_PACKED_DISABLE // _Pragma("pack(pop)")

// 内存屏障指令 用于同步数据与指令 防止工程开优化导致问题
#define ZF_DSB() __DSB()
#define ZF_ISB() __ISB()
#define ZF_DMB() __DMB()

// 文件定位
#define ZF_FILE_MESSAGE (__FILE__)
#define ZF_LINE_MESSAGE (__LINE__)
// -------------------------- IAR IDE 适配 --------------------------
#elif (IDE_ADS & IDE_TYPE)
// -------------------------- ADS IDE 适配 --------------------------
// ADS 使用 ADS* 工具链
// 内联函数修饰
#define ZF_INLINE static inline
#define ZF_WEAK __attribute__((weak))

// ADS 仅支持 2/4 字节对齐
#define ZF_PACKED
#define ZF_PACKED_ENABLE _Pragma("pack 2")
#define ZF_PACKED_DISABLE _Pragma("pack 0")

// 内存屏障指令 用于同步数据与指令 防止工程开优化导致问题
#define ZF_DSB()
#define ZF_ISB()
#define ZF_DMB()

// 文件定位
#define ZF_FILE_MESSAGE (__FILE__)
#define ZF_LINE_MESSAGE (__LINE__)
// -------------------------- ADS IDE 适配 --------------------------
#elif (IDE_MRS & IDE_TYPE)
// -------------------------- MRS IDE 适配 --------------------------
// MRS 使用 GNUC 工具链
// 内联函数修饰
#define ZF_INLINE static inline
#define ZF_WEAK __attribute__((weak))

// 内存对齐与紧凑结构
#define ZF_PACKED __attribute__((packed))
#define ZF_PACKED_ENABLE
#define ZF_PACKED_DISABLE

// 内存屏障指令 用于同步数据与指令 防止工程开优化导致问题
#define ZF_DSB()
#define ZF_ISB()
#define ZF_DMB()

// 文件定位
#define ZF_FILE_MESSAGE (__FILE__)
#define ZF_LINE_MESSAGE (__LINE__)
// -------------------------- MRS IDE 适配 --------------------------
#else
#error "IDE Type error!"
#endif
// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< Part   End <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

#endif
