/*
    File:           helloworld.c
    Description:    This is an easy driver module
    Author:         Cai
    Version:        v1.0    10Step25
*/

#include <linux/module.h>

static int arg = 0;

module_param(arg, int, S_IRUGO);    /* S_IRUGO表示参数可以被所有人读取，但不可以被改变 */

static int __init helloworld_init(void)
{
    pr_info("Hello world init.\n");
    pr_info("Param arg == %d\n", arg);
    return 0;
}

static void __exit helloworld_exit(void)
{
    pr_info("Hello world exit.\n");
}

module_init(helloworld_init);
module_exit(helloworld_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cai <sybstudy@yeah.net>");
MODULE_DESCRIPTION("This is an easy driver process");