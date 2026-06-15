# MSPM0G3507-LQFP64 引脚功能精简表

| 引脚号 | 引脚名称 | 模拟功能 / 电源 (ADC, OPA, COMP, VREF) | 主要数字通信外设 (UART, SPI, I2C, CAN) | 定时器 / 时钟 / 特殊功能 |
| :--- | :--- | :--- | :--- | :--- |
| **1** | PB13 | - | UART3 | TIMA0, TIMG12 |
| **2** | PB14 | - | SPI1, SPI0 | TIMG12, TIMA0 |
| **3** | PB15 | - | UART2, UART3, SPI1 | TIMG8, TIMG7 |
| **4** | PB16 | - | UART2, UART3, SPI1 | TIMG8, TIMG7 |
| **5** | PA12 | - | **CAN_TX**, UART3, SPI0 | TIMG0, TIMA0, FCC_IN |
| **6** | PA13 | COMP0_IN2- | **CAN_RX**, UART3, SPI0 | TIMG0, TIMA0 |
| **7** | PA14 | COMP0_IN2+ / A0_12 | UART0, UART3, SPI0 | TIMG12, CLK_OUT |
| **8** | PA15 | A1_0 / DAC_OUT / OPA0_IN / COMP | UART0, I2C1, SPI1 | TIMA1, TIMG8, TIMA0 |
| **9** | PA16 | A1_1 / OPA1_OUT | SPI1, I2C1 | TIMA1, TIMA0, FCC_IN |
| **10** | PA17 | A1_2 / OPA1_IN1- / COMP0_IN1- | UART1, SPI1, I2C1 | TIMA0, TIMG7, TIMA1 |
| **11** | PA18 | A1_3 / OPA1_IN1+ / COMP0_IN1+ | UART1, SPI1, I2C1 | TIMA0, TIMG7, TIMA1 |
| **12** | **PA19** | - | - | **SWDIO (调试)** |
| **13** | **PA20** | - | - | **SWCLK (调试)** |
| **14** | PB17 | A1_4 / COMP1_IN2- | UART2, SPI0, SPI1 | TIMA1, TIMA0 |
| **15** | PB18 | A1_5 / COMP1_IN2+ | UART2, SPI0, SPI1 | TIMA1, TIMA0 |
| **16** | PB19 | A1_6 / COMP2_IN1+ / OPA1_IN0+ | UART0 | TIMG8, TIMG7 |
| **17** | PA21 | A1_7 / COMP2_IN1- / VREF- | UART2, UART1 | TIMG8, TIMA0, TIMG6 |
| **18** | PA22 | A0_7 / GPAMP_OUT / OPA0_OUT | UART2, UART1 | TIMG8, TIMA0, TIMG6 |
| **19** | PB20 | A0_6 / OPA1_IN0- | SPI0, SPI1 | TIMA0, TIMG12, TIMA1 |
| **20** | PB21 | COMP2_IN0+ | SPI1 | TIMG8, TIMA1 |
| **21** | PB22 | COMP2_IN0- | SPI1 | TIMG8 |
| **22** | PB23 | - | SPI1 | TIMA_FAL0 |
| **23** | PB24 | A0_5 / COMP1_IN1+ | SPI0, SPI1 | TIMA0, TIMG12, TIMA1 |
| **24** | PA23 | COMP1_IN1- / VREF+ | UART2, SPI0 | TIMA0, TIMG0, TIMG6 |
| **25** | PA24 | A0_3 / OPA0_IN1- | UART2, UART3, SPI0 | TIMA0, TIMG0, TIMG7 |
| **26** | PA25 | A0_2 / OPA0_IN1+ | UART3, SPI1 | TIMG12, TIMA0 |
| **27** | PB25 | A0_4 | UART0, SPI0 | TIMA_FAL2 |
| **28** | PB26 | COMP1_IN0+ | UART0, SPI1 | TIMA0, TIMG6, TIMA1 |
| **29** | PB27 | COMP1_IN0- | SPI1 | TIMA0, TIMG6, TIMA1 |
| **30** | PA26 | A0_1 / COMP0_IN0+ / OPA0_IN0+ | **CAN_TX**, UART3, SPI1 | TIMG8, TIMG7 |
| **31** | PA27 | A0_0 / COMP0_IN0- / OPA0_IN0- | **CAN_RX**, SPI1 | TIMG8, TIMG7, RTC_OUT |
| **32** | **VCORE**| **核心电源引脚** | - | - |
| **33** | PA0 | - | UART0, **I2C0_SDA** | TIMA0, TIMG8 (5V容忍) |
| **34** | PA1 | - | UART0, **I2C0_SCL** | TIMA0, TIMG8 (5V容忍) |
| **35** | PA28 | - | UART0, I2C0_SDA | TIMA0, TIMG7, TIMA1 |
| **36** | PA29 | - | UART2, I2C1_SCL | TIMG8, TIMG6 |
| **37** | PA30 | - | UART2, I2C1_SDA | TIMG8, TIMG6 |
| **38** | **NRST** | **复位引脚** | - | - |
| **39** | PA31 | - | UART0, I2C0 | TIMA0, TIMG12, CLK_OUT |
| **40** | **VDD** | **主电源正极** | - | - |
| **41** | **VSS** | **主电源地 (GND)** | - | - |
| **42** | PA2 | ROSC | SPI0, SPI1 | TIMG8, TIMG7 |
| **43** | PA3 | LFXIN (低频晶振输入) | UART2, I2C1, SPI0 | TIMG8, TIMA0, TIMG7 |
| **44** | PA4 | LFXOUT (低频晶振输出) | UART2, I2C1, SPI0 | TIMG8, TIMA0, LFCLK_IN |
| **45** | PA5 | HFXIN (高频晶振输入) | SPI0 | TIMG8, TIMG0, TIMG6 |
| **46** | PA6 | HFXOUT (高频晶振输出) | SPI0 | TIMG8, TIMG0, HFCLK_IN |
| **47** | PB0 | - | UART0, SPI1 | TIMA1, TIMA0 |
| **48** | PB1 | - | UART0, SPI1 | TIMA1, TIMA0 |
| **49** | PA7 | - | - | CLK_OUT, TIMG8, TIMG7 |
| **50** | PB2 | - | UART3, UART1, UART2, I2C1 | TIMA0, TIMG6, TIMA1 |
| **51** | PB3 | - | UART3, UART1, UART2, I2C1 | TIMA0, TIMG6, TIMA1 |
| **52** | PB4 | - | UART1, UART3 | TIMA1, TIMA0 |
| **53** | PB5 | - | UART1, UART3 | TIMA1, TIMA0 |
| **54** | PA8 | - | UART1, UART0, SPI0 | TIMA0, TIMA1 |
| **55** | PA9 | - | UART1, UART0, SPI0 | TIMA0, TIMA1, RTC_OUT |
| **56** | PA10 | - | UART0, I2C0, I2C1, SPI0 | TIMA1, TIMG12, TIMA0 |
| **57** | PA11 | - | UART0, I2C0, I2C1, SPI0 | TIMA1, TIMA0 |
| **58** | PB6 | - | UART1, UART2, SPI1 | TIMG8, TIMG6, TIMA1 |
| **59** | PB7 | - | UART1, UART2, SPI1, SPI0 | TIMG8, TIMG6, TIMA1 |
| **60** | PB8 | - | UART1, SPI1 | TIMA0 |
| **61** | PB9 | - | UART1, SPI1 | TIMA0 |
| **62** | PB10 | - | - | TIMG0, TIMG8, TIMG6 |
| **63** | PB11 | - | - | TIMG0, TIMG8, TIMG6 |
| **64** | PB12 | - | UART3 | TIMA0 |

# 💡 核心引脚快速导览：
*   **电源/地/复位：** VDD (40), VSS (41), VCORE (32), NRST (38)
*   **调试接口 (SWD)：** SWDIO (12), SWCLK (13)
*   **默认I2C (BSL支持)：** PA0 (33, SDA), PA1 (34, SCL) —— *注：这组引脚支持5V容忍。*
*   **默认UART (BSL支持)：** PA10 (56, TX), PA11 (57, RX) 
*   **CAN-FD 接口复用：** PA12/PA13 (引脚5/6) 或 PA26/PA27 (引脚30/31)
*   **时钟晶振：** LFXIN/OUT (43/44, 低频), HFXIN/OUT (45/46, 高频)