/*
    This is a simple process for testing netlink, use in user space
    Author:     Cai
    Date:       12Step25
*/

/*
    Include files
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>

/*
    Macros
*/

#define NETLINK_EXAMPLE     (31)    // 必须和kernel那边使用的协议号一致
#define MAX_PAYLOAD         (1024)

/*
    Variables
*/

static struct sockaddr_nl src_addr = {};
static struct sockaddr_nl dst_addr = {};
static struct nlmsghdr *nlh = NULL;
static struct iovec iov = {};
static int sock_fd = 0;
static struct msghdr msg = {};
static const char *send_msg = "Hello kernel";

int main()
{
    printf("User space process started (PID: %d)\n", getpid());

    // 创建Netlink socket
    sock_fd = socket(AF_NETLINK, SOCK_RAW, NETLINK_EXAMPLE);
    if(sock_fd < 0)
    {
        perror("socket create");
        return -1;
    }

    // 配置源地址，即本进程的地址
    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;     // 不加入任何多播组

    // 绑定socket
    if(bind(sock_fd, (struct sockaddr*)&src_addr, sizeof(src_addr)) < 0)
    {
        perror("socket bind");
        close(sock_fd);
        return -1;
    }

    // 配置目标地址，即内核的地址
    memset(&dst_addr, 0, sizeof(dst_addr));
    dst_addr.nl_family = AF_NETLINK;
    dst_addr.nl_pid = 0;        // 内核pid永远是0
    dst_addr.nl_groups = 0;     // 不是多播

    // 创建并设置netlink消息头
    nlh = (struct nlmsghdr*)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if(!nlh)
    {
        perror("malloc");
        close(sock_fd);
        return -1;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_pid = getpid();      // 设置发送方pid
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), send_msg);

    // 设置IO向量和消息结构
    iov.iov_base = (void*)nlh;
    iov.iov_len = nlh->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void*)&dst_addr;
    msg.msg_namelen = sizeof(dst_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    // 发送消息到内核
    printf("Send message to kernel\n");
    if(sendmsg(sock_fd, &msg, 0) < 0)
    {
        perror("send message");
        free(nlh);
        close(sock_fd);
        return -1;
    }

    // 等待内核回复
    printf("Wait kernel response\n");
    if(recvmsg(sock_fd, &msg, 0) < 0)
    {
        perror("recv message");
        free(nlh);
        close(sock_fd);
        return -1;
    }

    // 打印回复
    char *reply = (char*)NLMSG_DATA(nlh);
    printf("Recv from kernel: %s\n", reply);

    free(nlh);
    close(sock_fd);
    return 0;
}