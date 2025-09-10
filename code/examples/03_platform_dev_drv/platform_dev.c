/*
    File:           platform_dev.c
    Description:    This is an easy platform driver module
    Author:         Cai
    Version:        v1.0    10Step25
*/

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/platform_device.h>
#include <linux/miscdevice.h>
#include <linux/mod_devicetable.h>

static int mydev_open(struct inode *inode, struct file *file);
static int mydev_close(struct inode *inode, struct file *file);
static long mydev_ioctl(struct file *file, unsigned int cmd, unsigned long args);
static int mydev_probe(struct platform_device *pdev);
static void mydev_remove(struct platform_device *pdev);

static struct file_operations mydev_fops = {
    .owner = THIS_MODULE,
    .open = mydev_open,
    .release = mydev_close,
    .unlocked_ioctl = mydev_ioctl,
};

static struct miscdevice mydev_miscdevice = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = "mydev",
    .fops = &mydev_fops,
};

static const struct of_device_id mydev_of_ids[] = {
    {.compatible = "syb,mydevkeys"},
    {}      // 必须{}表示结尾
};

static struct platform_driver mydev_platform_driver = {
    .probe = mydev_probe,
    .remove = mydev_remove,
    .driver = {
        .name = "mydevkeys",
        .of_match_table = mydev_of_ids,
        .owner = THIS_MODULE,
    },
};

int mydev_open(struct inode *inode, struct file *file)
{
    pr_info("mydev open() is called\n");
    return 0;
}

int mydev_close(struct inode *inode, struct file *file)
{
    pr_info("mydev close() is called\n");
    return 0;
}

long mydev_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
    pr_info("mydev ioctl() is called, cmd %d, args %ld\n", cmd, args);
    return 0;
}

int mydev_probe(struct platform_device *pdev)
{
    int ret = 0;

    pr_info("mydev probe is called\n");

    ret = misc_register(&mydev_miscdevice);
    if(ret != 0)
    {
        pr_info("Fail to register misc device.\n");
    }
    else
    {
        pr_info("Register misc device OK, got minor %d.\n", mydev_miscdevice.minor);
    }

    return ret;
}

void mydev_remove(struct platform_device *pdev)
{
    pr_info("mydev remove is called\n");

    misc_deregister(&mydev_miscdevice);
}

module_platform_driver(mydev_platform_driver);

MODULE_DEVICE_TABLE(of, mydev_of_ids);
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cai <sybstudy@yeah.net>");
MODULE_DESCRIPTION("This is an easy platform device driver process");