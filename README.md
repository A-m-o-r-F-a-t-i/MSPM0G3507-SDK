# MSPM0G3507 SDK

Lightweight firmware package for TI MSPM0G3507 development, combining device notes, TI DriverLib, TI Drivers, FreeRTOS files, SysConfig-based TI examples, and ZF wrapper examples.

## What Is Included

- Device documentation for MSPM0G3507 core resources, LQFP-48 pin mapping, and LQFP-64 pin mapping.
- Public headers under `include/` and matching implementations under `source/`.
- TI NoRTOS and RTOS examples under `TI_examples/`, including DriverLib, TI Drivers, FreeRTOS, and POSIX demos.
- ZF board examples under `ZF_examples/`, including GPIO, UART, ADC, PWM, PIT, EXTI, timer, flash, debug log, motor, display, and wireless demos.
- Context7 configuration in `context7.json` for documentation-aware code assistance.

## Project Layout

- `doc/`: Device notes, pin maps, datasheet, user manual, and documentation index.
- `include/core/ti/`: TI device, DriverLib, and TI Drivers public headers.
- `include/core/zf/`: ZF common, device, peripheral, and board wrapper headers.
- `include/rtos/`: FreeRTOS public headers and MSPM0 configuration.
- `source/core/ti/`: TI DriverLib and TI Drivers implementations.
- `source/core/zf/`: ZF wrapper implementations.
- `source/rtos/`: FreeRTOS kernel and MSPM0 port sources.
- `TI_examples/`: TI LP-MSPM0G3507 examples with SysConfig files and compiler project files.
- `ZF_examples/`: ZF core board and motherboard demos with user code and Keil projects.

## Documentation

- [`doc/README.md`](doc/README.md): Context7-friendly documentation index.
- [`doc/MSPM0G3507芯片核心信息.md`](doc/MSPM0G3507芯片核心信息.md): Core specifications, clocks, memory, power, and peripheral summary.
- [`doc/MSPM0G3507 LQFP-48 引脚功能表.md`](doc/MSPM0G3507%20LQFP-48%20引脚功能表.md): LQFP-48 pin functions.
- [`doc/MSPM0G3507 LQFP-64 引脚功能表.md`](doc/MSPM0G3507%20LQFP-64%20引脚功能表.md): LQFP-64 pin functions.
- `doc/mspm0g3507数据手册.pdf`: Vendor datasheet.
- `doc/mspm0g3507用户手册.pdf`: Vendor user manual.

## Example Families

- `TI_examples/nortos/driverlib/`: Peripheral-level NoRTOS examples for ADC12, DAC12, DMA, flash, GPIO, I2C, SPI, UART, timers, RTC, MCAN, OPA, COMP, SYSCTL, TRNG, and WWDT.
- `TI_examples/nortos/drivers/`: TI Drivers NoRTOS examples.
- `TI_examples/nortos/empty_mspm0g3507/`: Minimal LP-MSPM0G3507 NoRTOS project.
- `TI_examples/rtos/drivers/`: TI Drivers examples for FreeRTOS.
- `TI_examples/rtos/kernel/`: FreeRTOS kernel demos.
- `ZF_examples/Coreboard_Demo/`: ZF core board peripheral demos.
- `ZF_examples/Motherboard_Demo/`: ZF motherboard demos for keys, encoders, motor control, display, and wireless modules.

## SysConfig And DriverLib Usage

Use each example's `.syscfg` file and generated `ti_msp_dl_config.h` as the source of truth for pins, clocks, interrupts, and peripheral instances. Prefer TI DriverLib and existing ZF wrapper APIs over direct register access unless a low-level example explicitly requires it.

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

When adapting an example, keep generated SysConfig names unchanged, keep header/source pairs synchronized, and check the selected package pin map before assigning board pins.

## Development Notes

- Build TI examples from the compiler-specific subfolder when present, such as `gcc`, `iar`, `keil`, or `ticlang`.
- Treat generated `ti_msp_dl_config.c` and `ti_msp_dl_config.h` files as example artifacts; regenerate them from `.syscfg` when changing hardware configuration.
- Keep public interfaces in `include/` and implementations in `source/`.
- Avoid committing build outputs, object files, map files, binaries, IDE temporary files, or generated documentation noise.
