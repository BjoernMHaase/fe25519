Efficient field operations for Curve25519's prime (2^255)-19 for ARM Cortex M4 microcontrollers.

Repository for the field arithmetics and X25519 as reported in 
https://eprint.iacr.org/2018/286 
"AuCPace: Efficient verifier-based PAKE protocol tailored for the IIoT".

As one specific example on how to use the field operations, this repository also 
includes an efficient implementation for constant-time X25519 on the ARM Cortex M4. 

The X25519 scalar multiplication could be configured for "swap data" or "swap pointers". 
The "swap pointers" option is faster but must only be used if you are sure that your 
stack is residing in internal memory of the microcontroller. 

For this reason it's disabled by default and yield roughly 670k cycles instead of
the ~626k cycles from the paper. (Have a look at the scalarmult function in order
to change this setting).
 
The project is based on a "hello-world" program from Joost Rijneveld. Thank's also for Niels
Samwel for his help during setting up the project and re-structuring the code for publication.

The remarks below are almost a verbatime copy from Joost's hello world.

### Installation

This code assumes you have the [arm-none-eabi toolchain](https://launchpad.net/gcc-arm-embedded) installed and accessible. Confusingly, the tools available in the (discontinued) embedian project have identical names - be careful to select the correct toolchain (or consider re-installing if you experience unexpected behaviour). On most Linux systems, the correct toolchain gets installed when you install the `arm-none-eabi-gcc` (or `gcc-arm-none-eabi`) package. Besides a compiler and assembler, you may also want to install `arm-none-eabi-gdb`. On Linux Mint, be sure to explicitly install `libnewlib-arm-none-eabi` as well (to fix an error relating to `stdint.h`).
(Currently at November 2018 Ubuntu 18.04 seems to have some issues with incompatible versions of gcc and newlib.)

This project relies on the [libopencm3](https://github.com/libopencm3/libopencm3/) firmware. 
You need to checkout the sources and copy the main trunk directory into the libopencm3 subdirectory. 
Compile it (e.g. by calling `make lib` in one of the platform-specific directories) before attempting 
to compile any of the other targets.

The binary can be compiled by calling `make`. The binary can then be flashed onto the boards using [stlink](https://github.com/texane/stlink), as follows: `st-flash write usart.bin 0x8000000`. Depending on your operating system, stlink may be available in your package manager -- otherwise refer to their Github page for instructions on how to [compile it from source](https://github.com/texane/stlink/blob/master/doc/compiling.md) (in that case, be careful to use libusb-1.0.0-dev, not libusb-0.1).

### Hooking up an STM32 discovery board

Connect the board to your machine using the mini-USB port. This provides it with power, and allows you to flash binaries onto the board. It should show up in `lsusb` as `STMicroelectronics ST-LINK/V2`.

If you are using a UART-USB connector that has a PL2303 chip on board (which appears to be the most common), the driver should be loaded in your kernel by default. If it is not, it is typically called `pl2303`. On macOS, you will still need to [install it](http://www.prolific.com.tw/US/ShowProduct.aspx?p_id=229&pcid=41) (and reboot). When you plug in the device, it should show up as `Prolific Technology, Inc. PL2303 Serial Port` when you type `lsusb`.

Using dupont / jumper cables, connect the `TX`/`TXD` pin of the USB connector to the `PA3` pin on the board, and connect `RX`/`RXD` to `PA2`. Depending on your setup, you may also want to connect the `GND` pins.

### Troubleshooting

At some point the boards might behave differently than one would expect, to a point where simply power-cycling the board does not help. In these cases, it is useful to be aware of a few trouble-shooting steps.

#### Problems related to the tools

If you're using Ubuntu, a common issue when using stlink is an error saying you are missing `libstlink-shared.so.1`. In this case, try running [`ldconfig`](https://github.com/texane/stlink/blob/master/doc/compiling.md#fixing-cannot-open-shared-object-file).

#### Problems related to the board

First, check if all the cables are attached properly. For the boards supported in this repository, connect TX to `PA3`, RX to `PA2` and GND to `GND`. Power is typically supplied using the mini-USB connector that is also used to flash code onto the board. You could also use a uart interfaced at PC10. This is used on some Riscure boards for the STM32F407.

If the code in this repository does not appear to work correctly after flashing it on to the board, try pressing the `RST` button (optionally followed by re-flashing).

If you cannot flash new code onto the board, but are instead confronted with `WARN src/stlink-common.c: unknown chip id!`, try shorting the `BOOT0` and `VDD` pins and pressing `RST`. This selects the DFU bootloader. After that, optionally use `st-flash erase` before re-flashing the board.

If you cannot flash the code onto the board, and instead get `Error: Data length doesn't have a 32 bit alignment: +2 byte.`, make sure you are using a version of stlink for which [this issue](https://github.com/texane/stlink/issues/390) has been resolved. This affected L0 and L1 boards.
