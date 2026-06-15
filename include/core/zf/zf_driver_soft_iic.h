/*****************************************

* 文件名称          zf_driver_soft_iic
* 文件作用          软件 I2C 驱动（GPIO 模拟 I2C 通信、多字节读写）

****************************************/

#ifndef _zf_driver_soft_iic_h_
#define _zf_driver_soft_iic_h_

#include "zf_driver_gpio.h"

typedef struct
{
    uint32 scl_pin; // 用于记录对应的引脚编号
    uint32 sda_pin; // 用于记录对应的引脚编号
    uint8 addr;     // 器件地址 七位地址模式
    uint32 delay;   // 模拟 IIC 软延时时长
} soft_iic_info_struct;

void soft_iic_start(soft_iic_info_struct *soft_iic_obj);
void soft_iic_stop(soft_iic_info_struct *soft_iic_obj);

uint8 soft_iic_send_data(soft_iic_info_struct *soft_iic_obj, const uint8 data);
uint8 soft_iic_read_data(soft_iic_info_struct *soft_iic_obj, uint8 ack);

void soft_iic_write_8bit(soft_iic_info_struct *soft_iic_obj, const uint8 data);
void soft_iic_write_8bit_array(soft_iic_info_struct *soft_iic_obj, const uint8 *data, uint32 len);

void soft_iic_write_16bit(soft_iic_info_struct *soft_iic_obj, const uint16 data);
void soft_iic_write_16bit_array(soft_iic_info_struct *soft_iic_obj, const uint16 *data, uint32 len);

void soft_iic_write_8bit_register(soft_iic_info_struct *soft_iic_obj, const uint8 register_name, const uint8 data);
void soft_iic_write_8bit_registers(soft_iic_info_struct *soft_iic_obj, const uint8 register_name, const uint8 *data, uint32 len);

void soft_iic_write_16bit_register(soft_iic_info_struct *soft_iic_obj, const uint16 register_name, const uint16 data);
void soft_iic_write_16bit_registers(soft_iic_info_struct *soft_iic_obj, const uint16 register_name, const uint16 *data, uint32 len);

uint8 soft_iic_read_8bit(soft_iic_info_struct *soft_iic_obj);
void soft_iic_read_8bit_array(soft_iic_info_struct *soft_iic_obj, uint8 *data, uint32 len);

uint16 soft_iic_read_16bit(soft_iic_info_struct *soft_iic_obj);
void soft_iic_read_16bit_array(soft_iic_info_struct *soft_iic_obj, uint16 *data, uint32 len);

uint8 soft_iic_read_8bit_register(soft_iic_info_struct *soft_iic_obj, const uint8 register_name);
void soft_iic_read_8bit_registers(soft_iic_info_struct *soft_iic_obj, const uint8 register_name, uint8 *data, uint32 len);

uint16 soft_iic_read_16bit_register(soft_iic_info_struct *soft_iic_obj, const uint16 register_name);
void soft_iic_read_16bit_registers(soft_iic_info_struct *soft_iic_obj, const uint16 register_name, uint16 *data, uint32 len);

void soft_iic_transfer_8bit_array(soft_iic_info_struct *soft_iic_obj, const uint8 *write_data, uint32 write_len, uint8 *read_data, uint32 read_len);
void soft_iic_transfer_16bit_array(soft_iic_info_struct *soft_iic_obj, const uint16 *write_data, uint32 write_len, uint16 *read_data, uint32 read_len);

void soft_iic_sccb_write_register(soft_iic_info_struct *soft_iic_obj, const uint8 register_name, uint8 data);
uint8 soft_iic_sccb_read_register(soft_iic_info_struct *soft_iic_obj, const uint8 register_name);

void soft_iic_write_splicing_array(soft_iic_info_struct *soft_iic_obj, const uint8 *part1_data, uint32 part1_len, const uint8 *part2_data, uint32 part2_len);

void soft_iic_init(soft_iic_info_struct *soft_iic_obj, uint8 addr, uint32 delay, gpio_pin_enum scl_pin, gpio_pin_enum sda_pin);

#endif
