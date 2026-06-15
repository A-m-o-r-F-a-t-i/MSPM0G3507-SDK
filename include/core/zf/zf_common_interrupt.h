/*****************************************

* 文件名称          zf_common_interrupt
* 文件作用          中断管理（全局中断使能/禁止、中断优先级设置、中断初始化）

****************************************/

#ifndef _zf_common_interrupt_h_
#define _zf_common_interrupt_h_

#include "ti_msp_dl_config.h"

#include "zf_common_typedef.h"

void interrupt_global_enable(uint32 primask);
uint32 interrupt_global_disable(void);
void interrupt_enable(IRQn_Type irqn);
void interrupt_disable(IRQn_Type irqn);
void interrupt_set_priority(IRQn_Type irqn, uint8 priority);
void interrupt_init(void);

#endif
