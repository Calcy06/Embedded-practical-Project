#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <asm/ioctl.h>
#include <sys/socket.h>
#include <libubox/usock.h>
#include <libubox/uloop.h>
#include <uci.h> // 添加这个头文件

#define LOG(fmt, ...)               \
    printf("%s %d:   ",             \
           __FUNCTION__, __LINE__); \
    printf(fmt, ##__VA_ARGS__)

#define CFG_FILE_PATH "/home/firefly/uci_cfg/client" // 在此处修改路径为你的配置文件路径

char SERVER_IP[64];
char SERVER_PORT[64];

struct uloop_fd c_fd;        /*客户端接收数据句柄*/
struct uloop_timeout c_send; /*客户端定时发送句柄*/

char recv_buf[64]; /*数据接收缓冲区*/

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
            snprintf(SERVER_IP, sizeof(SERVER_IP), "%s", str);
            LOG("SERVER_IP: %s\n", SERVER_IP);

            str = uci_lookup_option_string(ctx, sec, "port");
            snprintf(SERVER_PORT, sizeof(SERVER_PORT), "%s", str);
            LOG("SERVER_PORT: %s\n", SERVER_PORT);
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

/*重连函数*/
static int reconnect()
{

    int type = USOCK_TCP | USOCK_NOCLOEXEC | USOCK_IPV4ONLY;
    c_fd.fd = usock(type, SERVER_IP, SERVER_PORT); /* create a linker socket*/
    if (c_fd.fd < 0)
    {
        perror("usock error");
        return -1;
    }

    uloop_fd_add(&c_fd, ULOOP_READ);
    uloop_timeout_set(&c_send, 2000); // 2s发一次

    return 0;
}
/*接收服务器数据*/
int clinet_recv(struct uloop_fd *sock, unsigned int events)
{
    int rev_len;
    rev_len = read(sock->fd, recv_buf, sizeof(recv_buf));
    if (rev_len > 0)
    {
        LOG("Recv:%s Len:%d \n", recv_buf, rev_len);
        memset(recv_buf, 0, sizeof(recv_buf));
        write(sock->fd, "#", 1);
    }
    else
    {
        /*此时判断断开连接，进入重连*/
        LOG("Recv  failed\n");
        goto error;
    }
    return 0;
error:
    if (sock->fd)
    {
        uloop_fd_delete(sock);
        close(sock->fd);
        sock->fd = -1;
        LOG("Close client fd\n");
        uloop_timeout_cancel(&c_send);
        if (reconnect() < 0)
        {
            LOG("ERROR: INIT FAILED\n");
        }
        else
        {
            LOG("CONNECT SUCCESS!!!\n");
        }
    }
    return 0;
}
/*定时向服务器发送数据*/
static void c_send_timer(struct uloop_timeout *timer)
{
    int ret;
    LOG("hello\n");
    ret = write(c_fd.fd, "hello", 5);
    if (ret < 0)
    {
        uloop_timeout_cancel(timer);
        LOG("Cancel send \n");
    }
    uloop_timeout_set(&c_send, 2000); // 2s发一次
}

static int client_init()
{

    int type = USOCK_TCP | USOCK_NOCLOEXEC | USOCK_IPV4ONLY;
    c_fd.fd = usock(type, SERVER_IP, SERVER_PORT); /* create a linker socket*/

    if (c_fd.fd < 0)
    {
        perror("usock");
        return -1;
    }
    /*客户端接收回调绑定*/
    c_fd.cb = clinet_recv;
    uloop_fd_add(&c_fd, ULOOP_READ);
    /*定时发送回调绑定*/
    c_send.cb = c_send_timer;
    uloop_timeout_set(&c_send, 2000); // 2s发一次

    return 0;
}
int main()
{
    read_uci_config(); // 在程序启动时读取UCI配置

    uloop_init();
    /*客户端初始化*/
    if (client_init() < 0)
    {
        LOG("ERROR: INIT FAILED\n");
    }
    else
    {
        LOG("CONNECT SUCCESS!!!\n");
    }
    uloop_run();

    close(c_fd.fd);
    uloop_done();
    return 0;
}
