#ifndef USOCK_H
#define USOCK_H

#include <libubox/uloop.h>

extern struct uloop_fd net_fd;

//网络初始化函数:创建TCP客户端并连接服务器
int net_init(const char* ip, const char* port);

#endif