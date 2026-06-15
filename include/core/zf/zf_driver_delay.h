/*****************************************

* 文件名称          zf_driver_delay
* 文件作用          延时函数驱动（毫秒/微秒级阻塞延时）

****************************************/

#ifndef _zf_driver_delay_h_
#define _zf_driver_delay_h_

#include "ti_msp_dl_config.h"

#include "zf_common_typedef.h"

void system_delay_ms(uint32 time);
void system_delay_us(uint32 time);

#endif
