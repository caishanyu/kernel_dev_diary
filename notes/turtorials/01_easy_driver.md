# easy_driver

从最简单的驱动程序讲起。对应的实验代码位于目录：[01_easy_driver](../../code/examples/01_easy_driver/)

## 驱动简介

嵌入式Linux系统设计的一个关键概念就是：**用户应用和底层硬件的隔离**。用户态的应用程序不允许直接访问外设寄存器、存储媒介、内存等。系统**使用内核驱动来访问硬件**、通过MMU管理内存，**应用程序运行在虚拟地址空间**

这样做是为了健壮性、可移植性

设备驱动可以表现为内核模块，可以是静态构建到内核镜像，或者动态加载到内核中

每个驱动都有一个`init()`和`exit()`函数，前者在驱动加载时调用，后者在卸载时调用。分别使用`module_init()`和`module_exit()`宏来指定入口和出口函数，让内核能够识别

还有一些宏用来指定模块的各个属性，比如`MODULE_LICENSE()`，用于设置许可证，一般必须设置，否则会报错“内核被污染”，部分功能被禁止

## 内核模块参数传递

大部分LKM（Linux Kernel Module, Linux可加载内核模块）都支持在加载、系统启动、运行时设置参数，可以是在模块加载时通过命令行提供参数，也可以在运行时通过**sysfs文件系统**读取参数

> /sys/module/<module_name> 目录是sysfs中专门用于暴露内核模块信息和控制接口的路径，其中文件和子目录主要用于用户空间和内核模块之间的交互，提供模块状态信息、配置选项和操作接口

模块参数通过`module_param(name, type, perm)`宏来实现，其中`name`为参数，`type`为类型，`perm`为参数属性

## 实验代码内容

实验代码`helloworld.c`实现了一个最简单的内核驱动程序，在模块加载/卸载时打印信息。

其中`__init`和`__exit`标识，是指定GCC编译属性，在加载/卸载结束后，自动释放相应内存，以节约内存空间，一般都要指定。

其中`pr_info("Hello world init.\n");`相当于`printk(KERN_INFO "Hello world init.\n")`。注意内核中打印不使用`printf`，**使用的是`printk`，可以指定打印信息的等级**

关于`Makefile`，可查看[Makefile](../../code/examples/01_easy_driver/Makefile)注释

在代码路径下执行`make`，可以生成我们需要的驱动`helloworld.ko`；执行`make clean`，清理编译产物

可以使用`sudo modinfo xxx.ko`来查看模块信息：

> filename:       /home/cai/share/kernel_dev_diary/code/examples/01_easy_driver/helloworld.ko
> description:    This is a easy driver process
> author:         Cai <sybstudy@yeah.net>
> license:        GPL
> srcversion:     C8B37CC37A39BCE9D18AC5E
> depends:        
> name:           helloworld
> vermagic:       6.12.25+rpt-rpi-2712 SMP preempt mod_unload modversions aarch64

使用`sudo insmod xxx.ko`来加载内核驱动

通过`sudo dmesg | tail`打印内核日志，可以看到`[ 4100.964997] Hello world init.`，说明我们的`helloworld_init()`被调用了

此时`ls /sys/module/helloworld`，可以看到存在该目录

使用`sudo rmmod xxx.ko`来卸载内核驱动，该例程可以看到`[ 4427.735856] Hello world exit.`，说明`helloworld_exit()`被调用了

此时`ls /sys/module/helloworld`，已经查找不到该目录了

---

该模块支持指定参数`arg`，在模块加载时指定，比如`sudo insmod helloworld.ko arg=40`

可以看到内核打印`Param arg == 40`

也可以在sysfs中查看参数

```bash
cai@raspberrypi: cat /sys/module/helloworld/parameters/arg
40
```