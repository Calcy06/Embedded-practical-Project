#include "uci_tcp.h"

#define CFG_FILE_PATH "/test/uci_cfg/client"

char server_ip[64];
char server_port[64];

struct uloop_fd c_fd;        /*客户端接收数据句柄*/
struct uloop_timeout c_send; /*客户端定时发送句柄*/

void read_uci_config()
{
    struct uci_context *ctx;
    struct uci_package *pkg;
    struct uci_section *sec;
    const char *str;

    ctx = uci_alloc_context();
    uci_load(ctx, CFG_FILE_PATH, &pkg);

    struct uci_element *ele;
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "client") == 0)
        {
            str = uci_lookup_option_string(ctx, sec, "ip");
            snprintf(server_ip, sizeof(server_ip), "%s", str);
            printf("server_ip: %s\n", server_ip);

            str = uci_lookup_option_string(ctx, sec, "port");
            snprintf(server_port, sizeof(server_port), "%s", str);
            printf("server_port: %s\n", server_port);
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

//重连函数
int reconnect()
{
    // 配置socket类型：TCP + 不自动关闭 + 仅IPv4
    int type = USOCK_TCP | USOCK_NOCLOEXEC | USOCK_IPV4ONLY;
    // 创建TCP客户端socket，连接服务器 192.168.4.94:60000
    // 返回的文件描述符保存到 c_fd.fd 中
    c_fd.fd = usock(type, server_ip, server_port);
    if (c_fd.fd < 0)
    {
        perror("usock error");
        return -1;
    }

    // 将客户端socket加入uloop事件循环，监听可读事件(socket 就是程序用来联网、收发网络数据的工具。)
    // 当服务器发消息来时，会触发回调函数
    uloop_fd_add(&c_fd, ULOOP_READ);
    // 重置定时器，10秒后再次尝试重连
    uloop_timeout_set(&c_send, 10000); 

    return 0;
}