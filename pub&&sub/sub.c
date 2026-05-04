#include <stdio.h>                // 标准输入输出头文件
#include <fcntl.h>                // 提供文件控制定义，如 fcntl、O_RDWR 等
#include <libubox/blobmsg_json.h> // 提供 JSON 格式的 blob 消息处理功能
#include <libubox/uloop.h>        // 提供事件循环功能
#include <libubus.h>              // 提供 ubus（进程间通信）功能
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <libubox/usock.h>
#include <sys/socket.h> // 提供 accept(), bind(), listen() 等函数
#include <unistd.h>     // 提供 read(), write(), close() 等函数
#include "uci_tcp.h"

static struct ubus_context *ctx; // ubus 上下文对象，用于与 ubus 系统交互
static uint32_t obj_id1;         // ubus 对象 ID
static uint32_t obj_id2;
static struct ubus_subscriber sub_event; // ubus 订阅者

// 回调函数，当收到 ubus 通知时执行
static int subscriber_cb(struct ubus_context *ctx, struct ubus_object *obj,
                         struct ubus_request_data *req, const char *method,
                         struct blob_attr *msg)
{
    if (obj->id == obj_id1)
    {
        printf("收到 gg 设备事件\n");
    }
    else if (obj->id == obj_id2)
    {
        printf("收到 sg 设备事件\n");
    }

    char *json_string = blobmsg_format_json(msg, true); // 将消息格式化为 JSON 字符串
    printf("notify_cb: %s\n", json_string);             // 打印接收到的消息

    write(c_fd.fd, json_string, strlen(json_string));

    free(json_string); // 释放 JSON 字符串内存

    return 0;
}

// 定时器回调函数：每隔5秒自动执行一次
void timer_cb(struct uloop_timeout *timer)
{
    printf("timer_cb\n");
    // 主动调用 ubus 服务：obj_id2 对象的 write_register 方法
    ubus_invoke(ctx, obj_id2, "write_register", NULL, NULL, NULL, 5000);
    // 重新设置定时器：5 秒后再次触发本函数（循环执行）
    uloop_timeout_set(timer, 5000);
}

int main()
{
    if (reconnect() != 0)
    {
        printf("连接服务器失败，程序退出\n");

        return -1;
    }

    struct uloop_timeout t;
    t.cb = timer_cb;

    uloop_init();             // 初始化事件循环
    ctx = ubus_connect(NULL); // 连接到 ubus
    ubus_add_uloop(ctx);      // 将 ubus 添加到事件循环

    sub_event.cb = subscriber_cb;              // 设置订阅者事件回调函数
    ubus_register_subscriber(ctx, &sub_event); // 注册订阅者

    const char *object1 = "gg";               // 要订阅的 ubus 对象名称
    ubus_lookup_id(ctx, object1, &obj_id1);   // 查找 ubus 对象 ID
    ubus_subscribe(ctx, &sub_event, obj_id1); // 订阅 ubus 对象

    const char *object2 = "sg";               // 要订阅的 ubus 对象名称
    ubus_lookup_id(ctx, object2, &obj_id2);   // 查找 ubus 对象 ID
    ubus_subscribe(ctx, &sub_event, obj_id2); // 订阅 ubus 对象

    uloop_timeout_set(&t, 5000);

    uloop_run();  // 运行事件循环
    uloop_done(); // 清理事件循环

    // 释放资源：取消订阅、断开 ubus 连接
    ubus_unregister_subscriber(ctx, &sub_event);
    ubus_free(ctx);

    return 0;
}