# MSPM0G3507 SDK

A lightweight SDK and documentation package for TI MSPM0G3507 microcontrollers.

## Features

- GPIO initialization examples
- UART transmit and receive examples
- SysTick delay examples
- Timer interrupt examples
- Basic RTOS integration notes
- Pin mapping notes for LQFP-48 and LQFP-64 packages

## Quick Start

```c
#include "ti_msp_dl_config.h"

int main(void)
{
    SYSCFG_DL_init();

    while (1) {
        DL_GPIO_togglePins(GPIO_LEDS_PORT, GPIO_LEDS_USER_LED_1_PIN);
        delay_cycles(32000000);
    }
}
```

## Project Layout

- `doc/`: MSPM0G3507 device notes, package pin maps, datasheet, and user manual.
- `include/`: CMSIS, TI DriverLib, TI Drivers, ZF wrappers, and RTOS headers.
- `source/`: DriverLib, TI Drivers, ZF wrappers, startup code, and RTOS sources.

## Documentation

- [`doc/README.md`](doc/README.md): Context7-friendly documentation index.
- [`doc/MSPM0G3507芯片核心信息.md`](doc/MSPM0G3507芯片核心信息.md): device specifications, clock, memory, power, and peripheral summary.
- [`doc/MSPM0G3507 LQFP-48 引脚功能表.md`](doc/MSPM0G3507%20LQFP-48%20引脚功能表.md): LQFP-48 pin functions.
- [`doc/MSPM0G3507 LQFP-64 引脚功能表.md`](doc/MSPM0G3507%20LQFP-64%20引脚功能表.md): LQFP-64 pin functions.
