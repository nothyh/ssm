STM32标准库项目生成工具
用户通过命令行/json文件/gui界面输入项目配置，工具生成项目文件
配置包括mcu,需要的外设
还需要下载标准库外设压缩包，和Keil pack
主要模块：
配置解析模块
mcu参数模块
下载文件
解压与管理模块
项目生成模块
    项目结构创建
    文件复制
    Makefile生成

用户输入应该包括项目路径，芯片型号和需要的外设

如何从芯片信号得到一些参数，包括，cpu, fpu, 一些变量，需要用到keil pack

先考虑简单的情况，所有参数有mcu.json 提供，包括，mcu全名，项目路径，需要的文件路径，需要的外设
参数可以由命令行提供，也可以有json提供，以最新的为准
需要的参数包括
-s --series
-l -- line
-sp -- stdpath
stdlib path
optional parameters:
-c --cortex
-f --fpu
--ff --fpufloat

-h --help

-p --project
-pe --peripherals  in the end of input
-d --debug
-q --quiet