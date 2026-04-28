#include "stdin.h"
#include "usock.h" //使用net_fd发送数据
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define BUF_SIZE 64

static char send_buf[BUF_SIZE];
static struct uloop_fd stdin_fd;

// 键盘输入回调函数
static void input_cb(struct uloop_fd *fd, unsigned int ev)
{
    int len = read(0, send_buf, BUF_SIZE - 1);
    if (len <= 0)
        return;

    if (send_buf[len - 1] == '\n')
        send_buf[len - 1] = '\0';

    if (!strcmp(send_buf, "exit"))
    {
        printf("\n程序退出\n");
        uloop_fd_delete(&net_fd); // 删除网络事件
        close(net_fd.fd);         // 关闭网络连接
        uloop_end();              // 终止事件循环
        return;
    }

    // 将用户输入的数据发送给服务器
    write(net_fd.fd, send_buf, strlen(send_buf));
    printf("\n【已发送】: %s\n【输入发送】: ", send_buf);
    fflush(stdout);
    memset(send_buf, 0, BUF_SIZE);
}

// 键盘输入初始化
void stdin_init()
{
    stdin_fd.fd = 0;
    stdin_fd.cb = input_cb;
    uloop_fd_add(&stdin_fd, ULOOP_READ);
}