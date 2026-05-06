#include <stdio.h>                // 标准输入输出头文件
#include <fcntl.h>                // 提供文件控制定义，如 fcntl、O_RDWR 等
#include <libubox/blobmsg_json.h> // 提供 JSON 格式的 blob 消息处理功能
#include <libubox/uloop.h>        // 提供事件循环功能
#include <libubus.h>              // 提供 ubus（进程间通信）功能
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include "read_gg.h"
#include "read_sg.h"
#include "write_register.h"
#include "free.h"

struct ubus_context *ctx;  // ubus 上下文对象，用于与 ubus 系统交互
static struct blob_buf b1; // blob 缓冲区，用于构建消息
static struct blob_buf b2;
static struct uloop_timeout notify_timer; // 定时器，用于定时通知

// 定义ubus方法(处理函数的统一格式)
typedef int (*ubus_handler_t)(struct ubus_context *ctx,
                              struct ubus_object *obj,
                              struct ubus_request_data *req, // ubus 请求数据结构
                              const char *method,
                              struct blob_attr *msg);

// GUANGGAN传感器数据获取方法（ubus调用的实际处理函数）
static int gg_get_data(struct ubus_context *ctx,
                       struct ubus_object *obj,
                       struct ubus_request_data *req,
                       const char *method,
                       struct blob_attr *msg)
{
    int data = read_gg();
    printf("pub中gg的data是%d\n", data);
    blob_buf_init(&b1, 0);              // 初始化 blob 缓冲区
    blobmsg_add_u32(&b1, "gg", data);   // 添加键值对："gg": data 到缓冲区
    ubus_send_reply(ctx, req, b1.head); // 将打包好的gg数据回复给客户端

    return 0;
}

// 定义 GUANGGAN 传感器的 ubus 方法集合
// 用于向 ubus 总线提供接口供外部调用（如：ubus call gg get_gg）
static struct ubus_method gg_methods[] = {
    {
        // ubus 调用方法名：执行 ubus call co get_gg 时触发 */
        .name = "get_gg",
        // 回调处理函数：ubus 收到请求后，会执行这个函数 */
        .handler = gg_get_data,
    },
};

// 定义gg传感器ubus对象类型
// 参数1：对象类型名称
// 参数2：对象方法集合
static struct ubus_object_type gg_type = UBUS_OBJECT_TYPE("gg", gg_methods);

// 定义 ubus 对象
static struct ubus_object gg_obj = {
    .name = "gg",                        // 对象名称
    .type = &gg_type,                    // 对象类型
    .methods = gg_methods,               // 对象支持的方法列表
    .n_methods = ARRAY_SIZE(gg_methods), // 方法数量
};

// 声光报警器的数据获取方法（ubus调用的实际处理函数）
static int sg_get_data(struct ubus_context *ctx,
                       struct ubus_object *obj,
                       struct ubus_request_data *req,
                       const char *method,
                       struct blob_attr *msg)
{
    int data = read_sg();
    printf("pub中sg的data是%d\n", data);
    blob_buf_init(&b2, 0);
    blobmsg_add_u32(&b2, "sg", data);
    ubus_send_reply(ctx, req, b2.head);

    return 0;
}

// 写声光报警器的寄存器
static int write_register_shengguang(struct ubus_context *ctx,
                                     struct ubus_object *obj,
                                     struct ubus_request_data *req,
                                     const char *method,
                                     struct blob_attr *msg)
{
    write_register(); // 调用写寄存器的函数，执行写操作

    return 0;
}

// 定义声光报警器的 ubus 方法集合
static struct ubus_method sg_methods[] = {
    {.name = "get_sg", .handler = sg_get_data},
    {.name = "write_register", .handler = write_register_shengguang},
};

// 定义声光报警器的ubus对象类型
static struct ubus_object_type sg_type = UBUS_OBJECT_TYPE("sg", sg_methods);

// 定义 ubus 对象
static struct ubus_object sg_obj = {
    .name = "sg",                        // 对象名称
    .type = &sg_type,                    // 对象类型
    .methods = sg_methods,               // 对象支持的方法列表
    .n_methods = ARRAY_SIZE(sg_methods), // 方法数量
};

// 定时器回调函数，用于定时发送通知
static void notify_timer_cb(struct uloop_timeout *timeout)
{
    // 推送 gg 数据
    int data = read_gg();
    blob_buf_init(&b1, 0);                          // 初始化 blob 缓冲区
    blobmsg_add_u32(&b1, "gg", data);               // 添加键值对："gg": data 到缓冲区
    ubus_notify(ctx, &gg_obj, "pub1", b1.head, -1); // 发送通知

    // 推送 sg 数据
    data = read_sg();
    blob_buf_init(&b2, 0);                          // 初始化 blob 缓冲区
    blobmsg_add_u32(&b2, "sg", data);               // 添加键值对："sg": data 到缓冲区
    ubus_notify(ctx, &sg_obj, "pub2", b2.head, -1); // 发送通知  
    
    // 重新设置定时器（保持5秒触发一次）
    uloop_timeout_set(timeout, 1000);
}

int main()
{
    uloop_init(); // 初始化事件循环
    if (modbus_serial_registers() != 0)
    {
        printf("modbus初始化失败\n");

        return -1;
    }

    ctx = ubus_connect(NULL); // 连接到 ubus
    if (!ctx)
    {
        printf("pub_main里面的ctx连接失败\n");
    }

    ubus_add_uloop(ctx); // 将 ubus 添加到事件循环

    // 设置文件描述符关闭时执行标志
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC);

    ubus_add_object(ctx, &gg_obj); // 将 gg 对象添加到 ubus
    ubus_add_object(ctx, &sg_obj); // 将 sg 对象添加到 ubus

    notify_timer.cb = notify_timer_cb;       // 设置定时器回调函数
    uloop_timeout_set(&notify_timer, 1000); // 设置定时器，1 秒后触发

    uloop_run(); // 运行事件循环

    free_all_resources(); // 释放所有资源
    uloop_done();         // 清理事件循环

    return 0;
}