# device tree overlay

本文档记录一下**设备树覆盖**

> 设备树（Device Tree）是嵌入式Linux系统中一种描述硬件配置的数据结构，它告诉操作系统当前设备上有什么硬件以及如何访问它们。而设备树覆盖（Device Tree Overlay） 则允许你在系统运行时动态地修改原有的设备树，比如添加新的设备节点、修改现有设备的属性等，而无需重新编译整个设备树或烧录整个系统镜像。这非常适合用来临时测试新驱动或硬件。

1. 编写设备树覆盖文件

创建一个新的.dts文件（例如 mytest.dts），用来描述你要添加的设备节点。

```dts
/dts-v1/;
/plugin/;  // 标明这是一个覆盖插件

/ {
    compatible = "raspberrypi,5-model-b", "brcm,bcm2712"; // 修改为你的树莓派型号对应的兼容性字符串，此处以树莓派4B为例
};
```

- 关键说明：
  - compatible 属性：必须与你编写的驱动代码中的 of_match_table 匹配，这样内核才能将驱动绑定到这个设备。
  - 节点名和属性：根据你的实际硬件和驱动需求填写。例如，如果你的设备连接到GPIO引脚，就需要定义 gpios 属性。你可以参考和中的例子。
  - 确保 target-path 正确指向你想要附加节点的位置。

2. 编译设备树覆盖文件

使用设备树编译器（dtc）将 .dts 文件编译成 .dtbo 文件。

```bash
dtc -I dts -O dtb -o mytest.dtbo mytest.dts
# -I dts: 指定输入文件格式为.dts
# -O dtb: 指定输出文件格式为dtb（实际输出为.dtbo）
```

如果系统尚未安装 dtc，可以通过命令 sudo apt-get install device-tree-compiler 安装。

3. 加载设备树覆盖

将编译好的 .dtbo 文件复制到 /boot/overlays/ 目录（树莓派通常使用这个路径）。

```bash
sudo cp mytest.dtbo /boot/overlays/
```

4. 启用设备树覆盖

编辑 /boot/firmware/config.txt 文件，在文件末尾添加一行，指定要加载的覆盖。

```
dtoverlay=mytest
```

5. 重启生效

6. 验证设备节点是否创建

重启后，检查新设备节点是否成功创建。你可以查看 /proc/device-tree/ 下是否有对应的节点：

```bash
ls /proc/device-tree/
```

或者使用 dmesg 查看内核启动日志，看是否有关于新设备的日志输出：

```bash
sudo dmesg | grep -i mytest
```

---

注意事项

- 内核配置：确保你的树莓派内核配置中启用了设备树覆盖支持（通常是 CONFIG_OF_OVERLAY）。树莓派官方系统通常默认开启。

- 驱动匹配：驱动代码中的 compatible 字符串必须与设备树覆盖中写的完全一致。

- 硬件资源：在设备树覆盖中正确指定硬件资源（如GPIO引脚、内存地址、中断号等），避免与其他设备冲突。错误配置可能导致系统不稳定。