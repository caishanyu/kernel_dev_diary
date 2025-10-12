/*
    This is a simple process for testing netlink, use in kernel space
    Author:     Cai
    Date:       12Step25
*/

/*
    Include files
*/

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <net/sock.h>
#include <linux/netlink.h>
#include <linux/skbuff.h>

/*
    Macros
*/

#define NETLINK_EXAMPLE     (31)

/*
    Function Declaration
*/

static void nl_recv_msg(struct sk_buff *skb);

/*
    Variables
*/

static struct sock *nl_sock = NULL;     // pointer to kernel netlink socket
static struct netlink_kernel_cfg cfg = {
    .input = nl_recv_msg,
};

/* 
    Functions Definition
*/

// recv user space message callback
void nl_recv_msg(struct sk_buff *skb)
{
    struct nlmsghdr *nlh = NULL;
    int pid = 0;
    struct sk_buff *skb_out = NULL;
    int msg_size = 0;
    char *reply_msg =  "Hello user, kernel recv your message";
    int res = 0;

    // 获取消息头部，其中pid为回复的地址
    nlh = (struct nlmsghdr*)skb->data;
    pid = nlh->nlmsg_pid;

    // 申请回复消息的空间
    msg_size = strlen(reply_msg);
    skb_out = nlmsg_new(msg_size, 0);
    if(!skb_out)
    {
        pr_err("Fail to allocate new skb\n");
        return;
    }

    // 设置消息
    nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
    NETLINK_CB(skb_out).dst_group = 0;  // 消息多播组设置为0，表示单播回复
    strncpy(NLMSG_DATA(nlh), reply_msg, msg_size);

    // 发送消息给用户空间
    res = nlmsg_unicast(nl_sock, skb_out, pid);
    if(res < 0)
    {
        pr_err("Error while sending msg to user\n");
    }

    pr_info("Send msg to user (PID: %d) done\n", pid);
}

static int __init my_module_init(void)
{
    pr_info("Initializing netlink example module\n");

    // 创建netlink socket
    nl_sock = netlink_kernel_create(&init_net, NETLINK_EXAMPLE, &cfg);
    if(!nl_sock)
    {
        pr_err("Error creating socket\n");
        return -ENOMEM;
    }

    pr_info("Create netlink socket successfully\n");
    return 0;
}

static void __exit my_module_exit(void)
{
    pr_info("Exiting netlink example module\n");

    if(nl_sock)
    {
        netlink_kernel_release(nl_sock);
        pr_info("Netlink socket released\n");
    }
}

module_init(my_module_init);
module_exit(my_module_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Cai");
MODULE_DESCRIPTION("This is a simple example of netlink kernel module");