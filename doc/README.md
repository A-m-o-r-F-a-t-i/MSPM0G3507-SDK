# MSPM0G3507 Documentation Index

This folder collects MSPM0G3507 device notes, package pin maps, and reference manuals for firmware development with TI DriverLib, SysConfig-generated initialization, and optional RTOS integration.

## Source Documents

- `MSPM0G3507芯片核心信息.md`: core device specifications, clocks, memory map, power modes, and peripheral counts.
- `MSPM0G3507 LQFP-48 引脚功能表.md`: complete LQFP-48 pin function table and dedicated power/debug/clock/reset pins.
- `MSPM0G3507 LQFP-64 引脚功能表.md`: compact LQFP-64 pin function table and common interface groups.
- `mspm0g3507数据手册.pdf`: vendor datasheet.
- `mspm0g3507用户手册.pdf`: vendor user manual.

## Device Summary

- CPU: Arm Cortex-M0+, up to 80 MHz.
- Flash: 128 KB with ECC.
- SRAM: 32 KB with ECC/parity protection.
- Voltage range: 1.62 V to 3.6 V.
- Temperature range: -40 degC to 125 degC.
- Memory protection: 8-region MPU.
- Main clocks: SYSOSC 4-48 MHz, SYSPLL up to 80 MHz, LFOSC 32.768 kHz, HFXT 4-48 MHz, LFXT 32 kHz.

## Peripheral Summary

- GPIO: 44 pins on LQFP-48, 60 pins on LQFP-64.
- UART: 4 instances; UART0 is extended, UART1-UART3 are main UART instances.
- I2C: 2 instances, up to 1 Mbit/s.
- SPI: 2 instances, up to 32 Mbit/s.
- CAN-FD: 1 instance.
- Timer: 2 TIMA advanced-control timers and 5 TIMG general-purpose timers.
- Analog: 2 ADC12 modules, 1 DAC12, 2 OPA modules, 3 comparators, 1 GPAMP.
- DMA: 7 channels.
- RTOS files: FreeRTOS headers in `include/rtos` and sources in `source/rtos`.

## Package Pin Notes

- LQFP-48 power/debug pins: VDD pin 6, VSS pin 7, VCORE pin 48, NRST pin 4, SWDIO PA19 pin 34, SWCLK PA20 pin 35.
- LQFP-48 clock pins: PA2/ROSC pin 8, PA3/LFXIN pin 9, PA4/LFXOUT pin 10, PA5/HFXIN pin 11, PA6/HFXOUT pin 12.
- LQFP-64 power/debug pins: VDD pin 40, VSS pin 41, VCORE pin 32, NRST pin 38, SWDIO PA19 pin 12, SWCLK PA20 pin 13.
- LQFP-64 BSL-friendly defaults: PA0 pin 33 as I2C0_SDA, PA1 pin 34 as I2C0_SCL, PA10 pin 56 as UART0_TX, PA11 pin 57 as UART0_RX.
- LQFP-64 CAN-FD options: PA12/PA13 on pins 5/6 or PA26/PA27 on pins 30/31.

## SDK Map

- `include/core/ti/driverlib`: TI DriverLib public headers for MSPM0 peripherals.
- `source/core/ti/driverlib`: TI DriverLib implementation files.
- `include/core/zf`: local ZF common, device, and peripheral wrapper headers.
- `source/core/zf`: local ZF wrapper implementations.
- `include/rtos`: FreeRTOS public headers and configuration.
- `source/rtos`: FreeRTOS kernel and MSPM0 port sources.

## SysConfig And DriverLib Usage

Use SysConfig-generated `ti_msp_dl_config.h` and `SYSCFG_DL_init()` as the board initialization entry point when present. Do not guess generated macro names; read the generated header from the target project before writing examples that reference GPIO ports, pins, IRQ names, timer instances, UART instances, or clock symbols.

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

For peripheral examples, prefer TI DriverLib functions and local wrapper APIs already present in the SDK. Keep interrupts short, place configuration constants in headers or SysConfig, and keep package-specific pin choices consistent with the LQFP-48 or LQFP-64 pin map.
