#include <stdio.h>                // 标准输入输出头文件
#include <fcntl.h>                // 提供文件控制定义，如 fcntl、O_RDWR 等
#include <libubox/blobmsg_json.h> // 提供 JSON 格式的 blob 消息处理功能
#include <libubox/uloop.h>        // 提供事件循环功能
#include <libubus.h>              // 提供 ubus（进程间通信）功能

static struct ubus_context *ctx;         // ubus 上下文对象，用于与 ubus 系统交互
static uint32_t obj_id;                  // ubus 对象 ID
static struct ubus_subscriber sub_event; // ubus 订阅者

// 回调函数，当收到 ubus 通知时执行
static int subscriber_cb(struct ubus_context *ctx, struct ubus_object *obj,
                         struct ubus_request_data *req, const char *method,
                         struct blob_attr *msg)
{
    char *json_string = blobmsg_format_json(msg, true); // 将消息格式化为 JSON 字符串
    printf("notify_cb: %s\n", json_string);             // 打印接收到的消息
    free(json_string);                                  // 释放 JSON 字符串内存
    return 0;
}

int main()
{
    uloop_init(); // 初始化事件循环

    ctx = ubus_connect(NULL);                                                // 连接到 ubus
    ubus_add_uloop(ctx);                                                     // 将 ubus 添加到事件循环
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC); // 设置文件描述符关闭时执行标志

    sub_event.cb = subscriber_cb;              // 设置订阅者事件回调函数
    ubus_register_subscriber(ctx, &sub_event); // 注册订阅者
    const char *object = "hello";              // 要订阅的 ubus 对象名称
    ubus_lookup_id(ctx, object, &obj_id);      // 查找 ubus 对象 ID
    ubus_subscribe(ctx, &sub_event, obj_id);   // 订阅 ubus 对象

    uloop_run();  // 运行事件循环
    uloop_done(); // 清理事件循环
    return 0;
}