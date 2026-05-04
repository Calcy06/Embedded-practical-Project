#include <stdio.h> // 标准输入输出头文件
#include <fcntl.h> // 提供文件控制定义，如 fcntl、O_RDWR 等
#include <libubox/blobmsg_json.h> // 提供 JSON 格式的 blob 消息处理功能
#include <libubox/uloop.h> // 提供事件循环功能
#include <libubus.h> // 提供 ubus（进程间通信）功能
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include "read_co/read_co.h"
#include "read_volume/read_volume.h"
#include "write_register/write_register.h"

struct ubus_context *ctx; // ubus 上下文对象，用于与 ubus 系统交互
static struct blob_buf b1; // blob 缓冲区，用于构建消息
static struct blob_buf b2;
static struct uloop_timeout notify_timer; // 定时器，用于定时通知
int f = 1;

// 一氧化碳报警器
static int co_get_data(struct ubus_context *ctx,
                           struct ubus_object *obj,
                           struct ubus_request_data *req,
                           const char *method,
                           struct blob_attr *msg)
{
    int data = read_co();
    blob_buf_init(&b1, 0);
    blobmsg_add_u32(&b1, "co", data);
    ubus_send_reply(ctx, req, b1.head);

    return 0;
}

static struct ubus_method co_methods[] = {
    {.name = "get_co", .handler = co_get_data},
};

static struct ubus_object_type co_type = 
              UBUS_OBJECT_TYPE("co", co_methods);

// 定义 ubus 对象
static struct ubus_object co_obj = {
    .name = "co", // 对象名称
    .type = &co_type, // 对象类型
    .methods = co_methods, // 对象支持的方法列表
    .n_methods = ARRAY_SIZE(co_methods), // 方法数量
};



// 声光报警器
static int volume_get_data(struct ubus_context *ctx,
                           struct ubus_object *obj,
                           struct ubus_request_data *req,
                           const char *method,
                           struct blob_attr *msg)
{
    int data = read_volume();
    blob_buf_init(&b2, 0);
    blobmsg_add_u32(&b2, "volume", data);
    ubus_send_reply(ctx, req, b2.head);

    return 0;
}
static int write_register_volume(struct ubus_context *ctx,
                           struct ubus_object *obj,
                           struct ubus_request_data *req,
                           const char *method,
                           struct blob_attr *msg)
{
    f = write_register(f);

    return 0;
}

static struct ubus_method volume_methods[] = {
    {.name = "get_volume", .handler = volume_get_data},
    {.name = "write_register", .handler = write_register_volume},
};

static struct ubus_object_type volume_type = 
              UBUS_OBJECT_TYPE("volume", volume_methods);

// 定义 ubus 对象
static struct ubus_object volume_obj = {
    .name = "volume", // 对象名称
    .type = &volume_type, // 对象类型
    .methods = volume_methods, // 对象支持的方法列表
    .n_methods = ARRAY_SIZE(volume_methods), // 方法数量
};




static void notify_timer_cb(struct uloop_timeout *timeout)
{
    int data = read_co();
    printf("pub中的data是%d\n", data);
    blob_buf_init(&b1, 0);
    blobmsg_add_u32(&b1, "co", data);
    ubus_notify(ctx, &co_obj, "pub", b1.head, -1); // 问题

    data = read_volume();

    blob_buf_init(&b2, 0);
    blobmsg_add_u32(&b2, "volume", data);
    ubus_notify(ctx, &volume_obj, "pub", b2.head, -1);

    uloop_timeout_set(timeout, 1000);
}

int main()
{
    uloop_init(); // 初始化事件循环
    if (modbus_serial_registers() != 0)
    {
        printf("modbus没有初始化成功\n");

        return -1;
    }
    ctx = ubus_connect(NULL); // 连接到 ubus
    if (!ctx)
    {
        printf("main里面的ctx连接失败\n");
    }

    ubus_add_uloop(ctx); // 将 ubus 添加到事件循环

    // 设置文件描述符关闭时执行标志
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC); 

    ubus_add_object(ctx, &co_obj); // 将 hello 对象添加到 ubus
    ubus_add_object(ctx, &volume_obj);

    notify_timer.cb = notify_timer_cb; // 设置定时器回调函数
    uloop_timeout_set(&notify_timer, 1000); // 设置定时器，1 秒后触发

    uloop_run(); // 运行事件循环
    free_sensor_list();
    free_serial_cfg();

    uloop_done(); // 清理事件循环

    return 0;
}