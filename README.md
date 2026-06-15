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
