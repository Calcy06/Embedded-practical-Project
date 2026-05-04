#ifndef _UCI_TCP_H_
#define _UCI_TCP_H_

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/socket.h>
#include <libubox/usock.h>
#include <libubox/uloop.h>
#include <uci.h> // 添加这个头文件

extern int reconnect();
extern struct uloop_fd c_fd;

#endif 