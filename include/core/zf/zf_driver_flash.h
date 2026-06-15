/*****************************************

* 文件名称          zf_driver_flash
* 文件作用          Flash 读写驱动（页/扇擦除、数据持久化存储）

****************************************/

#ifndef _zf_driver_flash_h_
#define _zf_driver_flash_h_

#include "zf_common_typedef.h"

// 以下宏定义为芯片固定设置 不允许用户修改
#define FLASH_BASE_ADDR (0x00016000) // 0x00008000(前32k地址)  前32k可擦除10w次 后96k擦除1w次
#define FLASH_MAX_PAGE_INDEX (2)
#define FLASH_MAX_SECTION_INDEX (6)
#define FLASH_PAGE_SIZE (0x00000400)                                // 1K byte
#define FLASH_SECTION_SIZE (FLASH_PAGE_SIZE * FLASH_MAX_PAGE_INDEX) // 2K byte
#define FLASH_OPERATION_TIME_OUT (0x0FFF)

#define FLASH_DATA_BUFFER_SIZE (FLASH_PAGE_SIZE / sizeof(flash_data_union)) // 自动计算每个页能够存下多少个数据

typedef union // 固定的数据缓冲单元格式
{
    float float_type;   // float  类型
    uint32 uint32_type; // uint32 类型
    int32 int32_type;   // int32  类型
    uint16 uint16_type; // uint16 类型
    int16 int16_type;   // int16  类型
    uint8 uint8_type;   // uint8  类型
    int8 int8_type;     // int8   类型
} flash_data_union;     // 所有类型数据共用同一个 32bit 地址

extern flash_data_union flash_union_buffer[FLASH_DATA_BUFFER_SIZE];

uint8 flash_check(uint32 sector_num, uint32 page_num);
uint8 flash_erase_page(uint32 sector_num, uint32 page_num);
void flash_read_page(uint32 sector_num, uint32 page_num, uint32 *buf, uint16 len);
uint8 flash_write_page(uint32 sector_num, uint32 page_num, const uint32 *buf, uint16 len);

void flash_read_page_to_buffer(uint32 sector_num, uint32 page_num);
uint8 flash_write_page_from_buffer(uint32 sector_num, uint32 page_num);
void flash_buffer_clear(void);

#endif
