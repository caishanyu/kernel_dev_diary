/*
    File:           char_dev_drv.c
    Description:    This is an easy char device driver module
    Author:         Cai
    Version:        v1.0    10Step25
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/miscdevice.h>

#define MAJOR_NUM   (202)
#define CLASS_NAME  "char_dev_class"
#define DEVICE_NAME "char_dev"

#define DEV_T_DYNAMIC   (1)
#define USE_MISC        (1)

static int char_dev_open(struct inode *inode, struct file *file);
static int char_dev_close(struct inode *inode, struct file *file);
static long char_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

#if !USE_MISC
static struct cdev char_dev = {};
static dev_t dev = 0;
static struct class *char_dev_class = NULL;
#endif

static const struct file_operations char_dev_fops = {
    .owner = THIS_MODULE,
    .open = char_dev_open,
    .release = char_dev_close,
    .unlocked_ioctl = char_dev_ioctl,
};

#if USE_MISC
static struct miscdevice char_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = DEVICE_NAME,
    .fops = &char_dev_fops,
};
#endif

static int __init char_dev_drv_init(void)
{
    int ret = 0;
#if !USE_MISC
#if DEV_T_DYNAMIC
    dev_t dev_no = 0;
    int major = 0;
#endif
    struct device *char_device = NULL;
#endif
    pr_info("char_dev_drv init start...\n");

#if USE_MISC

    /* 注册杂项设备到内核 */
    ret = misc_register(&char_miscdevice);
    if(ret != 0)
    {
        pr_info("register misc device fail\n");
        return ret;
    }
    pr_info("register misc device, got minor %i OK...\n", char_miscdevice.minor);

#else

#if DEV_T_DYNAMIC   /* 动态分配 */
    ret = alloc_chrdev_region(&dev_no, 0, 1, DEVICE_NAME);
    if(ret < 0)
    {
        pr_info("Unable to allocate major\n");
        return ret;
    }
    major = MAJOR(dev_no);  // 从dev_t类型变量拿出主设备号
    dev = MKDEV(major, 0);
#else   /* 静态分配 */
    /* 获取第一个设备标识 */
    dev = MKDEV(MAJOR_NUM, 0);
    /* 申请一个设备标识 */
    ret = register_chrdev_region(dev, 1, "char_dev");
    if(ret < 0)
    {
        pr_info("Unable to allocate major %d\n", MAJOR_NUM);
        return ret;
    }
#endif

    pr_info("char_dev number alloc OK, major %d...\n", MAJOR(dev));

    /* 初始化设备结构，将其注册到内核 */
    cdev_init(&char_dev, &char_dev_fops);
    pr_info("char_dev init cdev structure OK...\n");
    ret = cdev_add(&char_dev, dev, 1);
    if(ret < 0)
    {
        pr_info("Unable to add cdev\n");
        unregister_chrdev_region(dev, 1);
        return ret;
    }
    pr_info("char_dev add to kernel OK...\n");

    /* 注册设备类别到/sys/class中 */
    char_dev_class = class_create(CLASS_NAME);
    if(IS_ERR(char_dev_class))
    {
        unregister_chrdev_region(dev, 1);
        cdev_del(&char_dev);
        pr_info("Failed to register device class\n");
        return PTR_ERR(char_dev_class);
    }
    pr_info("register char_dev_class OK...\n");

    /* 创建设备文件，关联设备 */
    char_device = device_create(char_dev_class, NULL, dev, NULL, DEVICE_NAME);
    if(IS_ERR(char_device))
    {
        class_destroy(char_dev_class);
        cdev_del(&char_dev);
        unregister_chrdev_region(dev, 1);
        pr_info("Failed to mknod device\n");
        return PTR_ERR(char_device);
    }
    pr_info("create char device node OK...\n");

#endif

    pr_info("char_dev init OK\n");

    return 0;
}

static void __exit char_dev_drv_exit(void)
{
    pr_info("char_dev_drv exit start...\n");

#if USE_MISC
    misc_deregister(&char_miscdevice);
    pr_info("deregister misc device OK\n");
#else
    device_destroy(char_dev_class, dev);
    pr_info("remove device node OK...\n");

    class_destroy(char_dev_class);
    pr_info("remove device class OK...\n");

    cdev_del(&char_dev);
    pr_info("char_dev del OK...\n");

    unregister_chrdev_region(dev, 1);
    pr_info("unregister dev num region OK...\n");
#endif

    pr_info("char_dev exit OK\n");
}

int char_dev_open(struct inode *inode, struct file *file)
{
    pr_info("char_dev open.\n");
    return 0;
}

int char_dev_close(struct inode *inode, struct file *file)
{
    pr_info("char_dev close.\n");
    return 0;
}

long char_dev_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    pr_info("char_dev ioctl, cmd ==  %d, arg == %ld.\n", cmd, arg);
    return 0;
}

module_init(char_dev_drv_init);
module_exit(char_dev_drv_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cai <sybstudy@yeah.net>");
#if USE_MISC
MODULE_DESCRIPTION("This is an easy char device driver process, using misc device");
#else
MODULE_DESCRIPTION("This is an easy char device driver process");
#endif