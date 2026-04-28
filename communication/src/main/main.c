#include <stdio.h>
#include <uci.h>
#include <libubox/uloop.h>
#include "usock.h"
#include "stdin.h"
#include "read.h"

int main()
{
    read_uci();
    // 初始化uloop事件循环（核心框架）
    uloop_init();

    if (net_init(SERVER_IP, SERVER_PORT) < 0)
    {
        printf("连接服务器失败\n");
        return -1;
    }
    stdin_init();

    printf("     已连接,输入exit退出     \n【输入发送】: ");
    fflush(stdout);

    uloop_run();

    if (net_fd.fd >= 0)
        close(net_fd.fd);
    uloop_done();
    return 0;
}
