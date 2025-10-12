# rpi5

这里对树莓派5B做一个记录

## 内核编译

从github可以下载到树莓派使用的内核源码，笔者下载了最新版本linux-rpi-6.12.y

本地编译分为以下几个步骤

1. `sudo make bcm2712_defconfig`，5B使用的是bcm2712芯片。这一步是为了生成编译时使用的完整配置`.config`

```bash
cai@raspberrypi:~/share/rpi-linux/linux-rpi-6.12.y $ sudo make bcm2712_defconfig
  HOSTCC  scripts/basic/fixdep
  HOSTCC  scripts/kconfig/conf.o
  HOSTCC  scripts/kconfig/confdata.o
  HOSTCC  scripts/kconfig/expr.o
  LEX     scripts/kconfig/lexer.lex.c
  YACC    scripts/kconfig/parser.tab.[ch]
  HOSTCC  scripts/kconfig/lexer.lex.o
  HOSTCC  scripts/kconfig/menu.o
  HOSTCC  scripts/kconfig/parser.tab.o
  HOSTCC  scripts/kconfig/preprocess.o
  HOSTCC  scripts/kconfig/symbol.o
  HOSTCC  scripts/kconfig/util.o
  HOSTLD  scripts/kconfig/conf
#
# configuration written to .config
#
```

2. `sudo make -j4`，启动编译，`-j4`表示四核，整个编译大概需要40min
3. 编译产物主要包括内核镜像和设备树文件

## 引脚信息

在终端`pinout`可以打印出树莓派的一些引脚信息，如下

```
Description        : Raspberry Pi 5B rev 1.1
Revision           : d04171
SoC                : BCM2712
RAM                : 8GB
Storage            : MicroSD
USB ports          : 4 (of which 2 USB3)
Ethernet ports     : 1 (1000Mbps max. speed)
Wi-fi              : True
Bluetooth          : True
Camera ports (CSI) : 2
Display ports (DSI): 2
```

![pinout](../images/pinout.png)

图片可以对应到树莓派的板子上，其中一些之前不了解的概念

- SoC(System on Chip, 系统级芯片)：将计算机硬件的关键组件集成到单一芯片上，通常包含：CPU, GPU, MMU, 外设接口等
- CSI(Camera Serial Interface, 摄像头穿行接口)：专门为摄像头模块传输数据设计的串行接口