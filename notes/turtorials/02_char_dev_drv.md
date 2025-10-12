# char_dev_drv

## 字符设备驱动简介

Linux支持三种类型的设备，分别是**字符设备、块设备和网络设备**

字符设备的读写无需经过缓冲区，是最常见的设备类型；块设备读写以块为单位，支持随机读写；网络设备通过BSD套接字和网络子系统来访问

从应用程序的角度来看，字符设备就是一个文件，进程只知道一个`/dev`文件，通过系统调用`open`，`read`，`write`来进行文件操作

用户态调用系统调用，内核对应地指定相关的操作接口。为此，字符设备驱动需要**实现`file_operations`数据结构中的对应接口，并注册**。该数据结构定义在`include/linux/fs.h`中

内核模块**通过`copy_from_user()`和`copt_to_user()`来和用户态程序进行数据的交互**。

关于设备，在Linux中，**设备通过主设备号和从设备号来表示**，前者一般表示一个大类，后者表示具体编号，比如多个UART设备中的一个

### 字符设备相关API

编写字符设备代码需要引用头文件`<linux/cdev.h>`和`<linux/fs.h>`

内核中**使用`cdev`数据结构表示一个字符类型设备**，用于将设备注册到系统时

设备的**主设备号和次设备号使用`dev_t`类型表示**（unit32_t类型，高n位为主，其余为次），可以使用`MKDEV`宏生成

一组设备的**静态**分配和释放通过如下两个接口实现

```c
int register_chrdev_region(dev_t first, unsigned int count, char *name);
void unregister_chrdev_region(dev_t first, unsigned int count);
```

**动态**分配（推荐分配方式）则是：

```c
int alloc_chrdev_region(dev_t *dev, unsigned baseminor, unsigned count, const char *name);
// dev-输出参数，存放分配的第一个设备编号；baseminor-起始从设备号；count-请求从设备数量；name-关联的设备or驱动名称
```

分配完设备标识后，通过`cdev_init()`初始化字符设备，使用`cdev_add()`注册设备到内核，通过`cdev_del()`注销字符设备

### 使用设备文件系统创建设备文件

在Linux 2.6.32版本之前，必须使用`mknod`命令手动创建设备文件，此后版本引入sysfs虚拟文件系统，可以通过代码**自动创建设备节点**

以下API需要包含头文件`<linux/device.h>`

驱动使用以下内核API创建/销毁一个设备类别

```c
class_create(const char* name); /* 在/sys/class为设备创建一个类别 */
class_destroy(struct class *cls);    /* 移除设备类别 */
```

使用如下API创建设备节点文件

```c
device_create(struct class *cls, struct device *parent, dev_t devt, void *drvdata, const char *fmt, ...);        /* 在/dev/目录下创建设备文件 */
device_destroy(struct class *cls, dev_t dev);       /* 删除设备 */
```

### 杂项字符设备驱动

杂项框架是linux kernel提供的接口，该框架允许模块注册各自的从设备号。模块使用杂项设备的主设备号（官方分配`10`作为主设备号），避免单独为一个驱动申请一个新的主设备号，减少冲突可能性

使用杂项框架的设备，设备目录位于`/sys/class/misc/`目录下

杂项设备的数据结构定义在`include/linux/miscdevice.h`中的`miscdevice`结构

```c
struct miscdevice{
    int minor;          // 请求的从设备编号
    const char *name;   // 设备名称，可以在/proc/misc文件查看
    const struct file_operation *fops;
    struct list_head list;
    struct device *parent;
    struct device *this_device;
    const char *nodename;
    umode_t mode;
};
```

将`minor`成员设置为`MISC_DYNAMIC_MINOR`时，注册设备时会动态申请从设备号，否则使用申请的从设备号尝试注册

注册和注销杂项设备使用以下API

```c
int misc_regiseter(struct miscdevice *misc);
int misc_deregister(struct miscdevice *misc);
```

## 实验代码

实验代码位于[char_dev_drv](../../code/examples/02_char_dev_drv/)

其中：`DEV_T_DYNAMIC`宏用于控制是否动态分配设备号；`USE_MISC`宏用于控制是否使用杂项框架

执行`make`，得到目标`char_dev_drv.ko`和`ioctl_test`

可以看到，加载模块后，运行`ioctl_test`，内核打印存在相关打印

引入class和device相关API后，无需手动`mknod`创建设备文件，`ls /sys/class/char_dev_class`可以看到对应class，`ls /dev/char_dev`可以看到对应device节点

引入杂项设备后，`ls /sys/class/misc/char_dev`可以查看对应设备目录，`ls /dev/char_dev`可以看到对应设备节点

执行`make clean`清理编译产物