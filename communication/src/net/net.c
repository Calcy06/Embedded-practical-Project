#include "usock.h"
#include <libubox/usock.h>
#include <stdio.h>
#include <unistd.h> // Unix标准接口（read/close文件操作）
#include <string.h>

#define BUF_SIZE 64

// 网络事件句柄（监听）
struct uloop_fd net_fd;
// 静态接收缓冲区:存储服务器发送的数据
static char recv_buf[BUF_SIZE];

// 网络接收数据回调函数
// sock 事件句柄（包含socket文件描述符）
// ev 触发事件的类型
static void recv_cb(struct uloop_fd *sock, unsigned int ev)
{
    // socket读取数据到缓冲区
    int len = read(sock->fd, recv_buf, BUF_SIZE - 1);

    if (len > 0)
    {
        recv_buf[len] = '\0';
        printf("\n服务器: %s 输入发送：", recv_buf);
        fflush(stdout);                // 刷新输出缓冲区
        memset(recv_buf, 0, BUF_SIZE); // 清空接收缓冲区
    }
    else
    {
        printf("服务器断开连接,程序退出\n");
        uloop_fd_delete(sock); // 删除网络监听事件
        close(sock->fd);       // 关闭socket连接
        sock->fd = -1;         // 标记文件描述符无效
        uloop_end();           // 终止uloop事件循环
    }
}

// 网络初始化，连接TPC服务器
int net_init(const char *ip, const char *port)
{
    // 设置socket类型：TCP协议 + 仅使用IPv4
    int type = USOCK_TCP | USOCK_IPV4ONLY;
    net_fd.fd = usock(type, ip, port);

    if (net_fd.fd < 0)
        return -1;

    // 绑定数据接收回调函数
    net_fd.cb = recv_cb;
    // 将网络句柄加入uloop，监听、读事件（接收数据）
    uloop_fd_add(&net_fd, ULOOP_READ);

    return 0;
}
