# rpi5

这里对树莓派5B做一个记录

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