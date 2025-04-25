the std lib do not provide a linker script, but you can use the one genreated by stm32cubemx which is the most convenient method.

use the startup file in gcc dir, not arm dir which is used by keil armasm assembler, otherwise the gcc can not compile it

stm32xx_it.c/h  stm32xx_conf.h find them in the stdlib , not keil pack which are used by keil

two variable to define in makefile
density: STM32F10X_MD for stm32f10x.h
to use assert_param need include stm32f10x_conf.h in stm32f10x.h so need define a variable use_stdper



for user input
gui 
cli

core
    + get user input
        + construct the MCU class, get mcu information
    + fetch pack, then unzip 
    + process user input and copy files
    + gernerate makefile

ref:
https://open-cmsis-pack.github.io/devtools/buildmgr/latest/cpackget.html
https://github.com/Open-CMSIS-Pack/devtools/releases
https://sadevicepacksdqaus.blob.core.windows.net/idxfile/index.pidx