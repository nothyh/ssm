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
不应该在命令行输入这些参数,太多，易错
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
使用json传递参数
-j --json


ssm_config.json
keil_pack:
如果提供了pack路径，不下载，直接解压，然后解析
否则，到~/ssm下寻找, 如果找到 解压，解析
否则下载到指定路径或~/ssm 或当前文件夹, 解压，解析

解压用
解压：解压到tmp下随机
解析：解析后，参数传递给all_config

stdlib:  一个类，包含许多文件路径
解压library到项目路径,
解压project下的部分到项目路径,
设置各个变量

makefile:一个类，包含需要替换的内容
cpu ,fpu 来自json
source, include来自makefiel


