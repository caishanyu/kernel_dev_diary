# platform device driver

本章介绍平台设备驱动

首先一个问题是，什么是平台设备驱动？前边已经实现的字符设备驱动，只需要调用`open()`来打开对应的设备文件，就可以通过`file_operations`文件操作接口来控制硬件

但是这种方式存在一个问题，就是设备信息和驱动代码耦合在一块了。在未来的驱动程序中，各种硬件寄存器地址随处可见，一旦硬件信息发生变化，代码就需要修改，这样是很麻烦的

为了解决这种问题，linux提出了设备驱动模型，引入总线概念来分离驱动代码和设备信息

对于I2C、SPI、USB这些常见的物理总线，内核会自动创建与之相对应的驱动总线，但还存在很多结构简单的设备，没有对应物理总线，例如rtc、led、蜂鸣器等，**为了使这些设备也能够遵循设备驱动模型，内核引入了虚拟总线——平台总线（platform bus）**

## 平台设备驱动简介

连接到平台总线上的设备，需要被**静态发现**而不是被动态探测，例如写到设备树里

**平台设备驱动程序负责在设备模型核心中实例化并注册`platform_driver`数据结构实例**，驱动提供`probe()`和`remove()`方法，用于匹配到设备、设备离开时的回调；**`probe`函数在总线驱动程序和设备匹配上时被调用**，在其中一般进行设备的初始化

平台驱动程序需要使用`platform_driver_register()`函数来将驱动程序注册到平台核心，使用`platform_driver_unregister()`函数来注销

当模块被加载时，使用`platform_driver_register`将自己注册到平台总线驱动中，当该驱动`struct of_device_id`数据结构中的某个`compatible`属性值和设备树（DT）中的某个`compatible`完全匹配时，驱动的`probe`函数将被调用。

因此需要在模块的`init`中调用驱动注册

此外可以**使用`module_platform_driver()`宏，该宏会调换掉`module_init`和`module_exit`。对于不需要在init和exit中进行复杂操作的设备驱动，可以简化代码**

### 相关数据结构和API

`struct of_device_id`定义在`include/linux/mod_devicetable.h`中，用于驱动匹配设备树中的设备，其中最重要的是`compatible`属性，是一个字符串，需要和设备树中设备的`compitable`属性完全匹配

```c
struct of_device_id{
    char name[32];
    char type[32];
    char compatible[128];
    const void *data;
};
```

`MODULE_DEVICE_TABLE(type, table_name)`用于声明设备驱动和硬件设备之间的匹配表，帮助内核在加载模块时自动关联驱动程序和设备，使用**值`of`表示设备树匹配表类型**

`platform_driver_register`函数使用`struct platform_driver`数据结构，其中比较重要的成员：

```c
struct platform_driver{
    int (*probe)(struct platform_device*);
    int (*remove)(struct platform_device*);
    void (*shutdown)(struct platform_device*);
    /* 以下两个支持低功率管理 */
    int (*suspend)(struct platform_device*, pm_message_t);
    int (*resume)(struct platform_device*);

    struct device_driver driver;
}
```

其中`struct device_driver`用于表示一个设备驱动程序，比较重要的有如下，我们在代码中也初始化了这几个属性

```c
struct device_driver{
    const char *name;
    struct module   *owner;     // 模块所属（如 THIS_MODULE）
    const struct of_device_id *of_match_table;  // 设备匹配表
};
```

再看看`module_platform_driver()`宏，展开后等价于：

```c
static int __init my_driver_init(void) {
    return platform_driver_register(&my_platform_driver);
}

static void __exit my_driver_exit(void) {
    platform_driver_unregister(&my_platform_driver);
}

module_init(my_driver_init);
module_exit(my_driver_exit);
```

会自动关联init和exit函数，进行驱动的注册和注销，可以大大简化代码

### 实验代码

实验代码位于路径[03_platform_dev_drv](../../code/examples/03_platform_dev_drv/)

尴尬的是需要更新设备树进行编译，再验证，工位的电脑应该是不支持我下载源码等操作，回家再看看~

TODO: 09/15 使用dts overlay失败，看起来内核加载就没有显示这个新增的设备

## 硬件相关知识

在之后的开发中，需要操作不同的设备，比如LED，按钮，I2C设备等，因此有必要了解硬件相关知识

首先是**引脚**，引脚表示电信号的物理输入/输出接线，每个引脚（也叫焊点）都有制造商提供的引脚名称，在原理图中可以查看不同引脚的名称。

一般处理器功能很丰富，但引脚数量是有限的，虽然单个引脚在特定时刻只能完成单一功能，但我们可以内部对其配置，使其完成不通的功能，称为**引脚复用**

笔者使用的开发板为树莓派5，在终端输入`pinout`可查看引脚简略信息，输出如下，有颜色的图片单独放出来，比较好区分引脚信息

```bash
cai@raspberrypi:~ $ pinout
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

For further information, please refer to https://pinout.xyz/
```

![pinout](../images/pinout.png)

## 引脚控制子系统

在旧的Linux引脚复用代码中，每个体系结构都有自己的引脚复用代码，引脚复用功能必须在SoC级别实现，且设备驱动程序无法调用

新的Pinctrl子系统目的在于解决这个问题，实现位于`drivers/pinctrl`中，提供如下功能：

- 用于注册pinctrl驱动程序的API，例如列出引脚列表、引脚功能等，主要是SoC特定的驱动程序，提供引脚复用的能力
- 被设备驱动程序用来请求对某些引脚复用的API
- 与Soc GPIO驱动程序交互

Pinctrl子系统处理以下功能，实现代码在`drivers\pinctrl\bcm\pinctrl-bcm2835.c`中（RPI 5B）

- 对可控制的引脚进行枚举、命名
- 引脚复用
- 引脚配置，上拉下拉、开漏输出等

其中`bcm2835_pinctrl_probe`接口比较关键，其中解析了设备树，然后对涉及的寄存器设置值等

## 设备树引脚控制器绑定

引脚控制器允许处理器与多个功能块共享一个焊点，通过多路复用焊点IO信号来实现

由于不同模块对焊点的设置不一致，因此引脚控制器也需要对焊点设置参数来控制

与其它硬件模块一样，引脚控制器在设备树中表示为一个节点，功能模块也是在设备树中表示为一个节点，其中需要描述引脚的配置，静态or动态皆可

对于每个客户端设备

- 分别为**每个引脚状态分配一个整数ID**，从0开始且连续
- 对于每个状态ID，存在唯一的属性来定义引脚配置
- 对于每个状态，分配一个**状态名称**。使用状态名称时，需要另一个属性存在，用以将名称映射到ID

客户端设备树需要绑定其状态集合，需要在其设备树节点中定义，必须定义的属性如下：

- `pinctrl-0`：phandle列表，每个phandle指向一个引脚配置节点，被引用的必须是所配置的引脚控制器的子节点，

可选的属性如下：

- `pinctrl-1`：phandle列表
- [...]：phandle列表
- `pinctrl-n`：phandle列表
- `pinctrl-names`：分配给引脚状态的名称列表，列表条目0对应状态ID为0的名称，以此类推

一个示例如下：

```dts
/* a client device requiring named states */
device {
    pinctrl-names = "active", "idle";
    pinctrl-0 = <&state_0_node_a>;
    pinrctl-1 = <&state_1_node_a & state_1_node_b>;
};
```

引脚控制器需要包含客户端设备引用的引脚配置节点

```dts
pincontroller {
    ... // elided other properties
    state_0_node_a {
        ...
    };
    state_1_node_a {
        ...
    };
    state_1_node_b {
        ...
    };
};
```

## GPIO控制器驱动

在内核中，每个GPIO控制器驱动都需要包含头文件`linux/gpio/driver.h`，其中定义了用于开发GPIO驱动的不同数据结构

GPIO控制器驱动内部，不同GPIO由硬件编号标识，此外还有一个全局标号，兼容旧版本的GPIO接口

GPIO控制器驱动的主要数据结构是`gpio_chip`，其中包含一些回调函数成员，用于访问GPIO等，具体查看`linux/gpio/driver.h`中的定义

以下记录如何使用基于描述符的GPIO接口，这是新版本linux推荐的做法，使用`linux/gpio/consumer.h`头文件。这些接口都带有`gpiod_`前缀，而`gpio_`是旧版本的前缀，强烈建议补药使用旧版本

1. 获取和释放GPIO

`struct gpio_desc* devm_gpiod_get(struct device *dev, const char *con_id, enum gpiod_flag flags)`

返回`gpio_desc`结构指针，指向**GPIO描述符**，输入（1）GPIO设备（2）GPIO功能（3）可选的GPIO初始化标志

该函数有个变体：`struct gpio_desc* devm_gpiod_get—_index(struct device *dev, const char *con_id, unsigned int idx, enum gpiod_flag flags)`

多个一个idx参数，用于访问特定GPIO功能内部定义的多个GPIO中的一个，使用到`of_find_gpio()`函数，在设备树中查找con_id和idx对应的功能

此外还有不带`devm_`前缀的版本

`flags`参数可以指定GPIO方向和初始值，比较重要的如下：

- `GPIO_ASIS`或者`0`：根本不初始化GPIO，后续使用某个专门函数来设置
- `GPIO_IN`：输入
- `GPIO_OUT_LOW`：输出低电平
- `GPIO_OUT_HIGH`：输出高电平

使用如下接口来释放GPIO描述符

`void devm_gpiod_put(struct device *dev, struct gpio_descs *descs)`

2. 使用GPIO

编写需要控制GPIO的驱动程序时，需要首先指定GPIO方向，可以使用步骤1里`devm_gpiod_get`带方向的参数，也可以使用参数`0`，然后如下API设置

```c
int gpiod_direction_input(struct gpio_desc *desc)
int gpiod_direction_output(struct gpio_desc *desc, int value)
```

调用成功返回`0`，否则负数。

使用如下API可以在**原子上下文**内访问GPIO

```c
int gpiod_get_value(const struct gpio_desc *desc)
void gpiod_set_value(struct gpio_desc *desc, int value)
```

注意**驱动程序不关注具体物理线路，set相关函数均使用逻辑值，将`value`解释为是否有效**。比如某个GPIO被设置了低电平有效，那么set 1则将其输出设置为低电平

3. GPIO映射到中断

中断可以通过GPIO触发，可以使用如下API获取GPIO对应的IRQ号

```c
int gpiod_to_irq(const struct gpio_desc *des)
```

如果映射失败，返回负数错误码。获取的irq可以直接传递给`request_irq`和`free_irq`函数，后续介绍相关函数

4. GPIO设备树

在设备树中，GPIO被映射到设备及其功能，用来描述哪些设备使用了哪些GPIO。通过`<function>-gpios`属性来定义，其中`<function>`被Linux驱动程序通过`gpiod_get`函数来请求

比如下面的例子：

```dts
foo_device{
    compitable = "acme,foo";
    ...
    led-gpios = <&gpioa 15 GPIO_ACTIVE_HIGH>,   // red
                <&gpioa 16 GPIO_ACTIVE_HIGH>,   // green
                <&gpioa 17 GPIO_ACTIVE_HIGH>;   // blue
    
    power-gpios = <&gpiob 1 GPIO_ACTIVE_LOW>;
};

// gpioa gpiob 是特定gpio控制器节点的phandle
// 15 16 17 1 是GPIO的线编号
// GPIO_ACTIVE_HIGH GPIO_ACTIVE_LOW 为GPIO flag之一，表示有效电平
```

驱动程序中可以如下使用：

```c
struct gpio_desc *red, *green, *blue, *power;
red = gpiod_get_index(dev, "led", 0, GPIO_OUT_HIGH);
green = gpiod_get_index(dev, "led", 1, GPIO_OUT_HIGH);
blud = gpiod_get_index(dev, "led", 2, GPIO_OUT_HIGH);
power = gpiod_get_index(dev, "power", 0, GPIO_OUT_LOW);
```

## 内核态和用户态之间交换数据

Linux禁止进程直接访问或操作内核数据结构和服务，通过将整个内存分割为内核态和用户态两个逻辑部分来实现

系统调用在内核态中实现，并且相应的处理程序通过用户态中的API来调用

进程执行系统调用时，内核在调用者的进程上下文执行；内核相应中断时，内核中断处理程序异步运行在中断上下文

内核驱动程序无法直接从内核访问进程地址空间，需要通过以下宏/函数来实现

1. 单个变量访问：

- `get_user(type val, type *address)`：内核变量`val`获取用户态指针`address`指向的值
- `put_user(type val, type *address)`：将用户态指针`address`指向的变量值设置为内核变量`val`的内容

2. 数组访问

- `unsigned long copy_to_user(void __user *to, const void *from, unsigned long n)`：将内核态中`from`引用的地址起始，复制`n`个字节到`to`的用户态地址
- `unsigned long copy_from_user(void *to, const void __user *to, unsigned long n)`：用户态复制到内核态

## MMIO（内核映射I/O）设备访问

对于外围设备的控制，一般是通过读取/写入设备的寄存器来实现。通常设备具有多个寄存器，通过内存地址空间（MMIO）或者I/O地址空间（PIO）的连续地址来访问这些寄存器

- MMIO：主存和IO设备使用相同的总线地址；使用常规指令来访问IO设备；linux支持绝大部分架构
- PIO：主存和IO设备使用不同地址空间；使用特殊CPU指令来访问IO设备

**RPI使用的是MMIO方式**，重点关注

**Linux驱动无法直接访问物理IO地址，需要MMU来映射**。默认情况下，IO内存并未映射到虚拟内存中

可以使用以下API来获取IO虚拟地址

```C
void __iomem *ioremap(phys_addr_t offset, unsigned long size)   // 将offset起始的地址，连续size个字节的内存映射，返回给程序
void iounmap(void *address) // 解除映射
```

使用`ioremap`获得的指针可以直接解引用，若无法映射，该函数返回`NULL`

此外还有`devm_ioremap()和devm_iounmap()`函数，定义在`include/linux/io.h`中，在`lib/devres.c`中实现。用于在驱动程序和设备之间进行/解除映射。目前比较推荐这种做法，可以简化驱动程序代码和错误管理

```C
void __iomem *devm_ioremap(struct device *dev, resource_size_t offset, unsigned long size)
void devm_iounmap(struct device *dev, void _iomem *addr)
```

直接解引用`devm_ioremap`返回的指针是危险的，可能导致缓存和同步问题。**内核提供读写的API**

```c
unsigned read[bwl](void *addr)
void write[bwl](unsigned val, void *addr)
```