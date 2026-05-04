#include <stdio.h> // 标准输入输出头文件
#include <fcntl.h> // 提供文件控制定义，如 fcntl、O_RDWR 等
#include <libubox/blobmsg_json.h> // 提供 JSON 格式的 blob 消息处理功能
#include <libubox/uloop.h> // 提供事件循环功能
#include <libubus.h> // 提供 ubus（进程间通信）功能

struct ubus_context *ctx; // ubus 上下文对象，用于与 ubus 系统交互
static struct blob_buf b; // blob 缓冲区，用于构建消息
static struct uloop_timeout notify_timer; // 定时器，用于定时通知

// 定义 ubus 方法
static struct ubus_method hello_methods[] = {};

// 定义 ubus 对象类型
static struct ubus_object_type hello_type = UBUS_OBJECT_TYPE("hello", hello_methods);

// 定义 ubus 对象
static struct ubus_object hello_obj = {
    .name = "hello", // 对象名称
    .type = &hello_type, // 对象类型
    .methods = hello_methods, // 对象支持的方法列表
    .n_methods = ARRAY_SIZE(hello_methods), // 方法数量
};

// 定时器回调函数，用于定时发送通知
static void notify_timer_cb(struct uloop_timeout *timeout)
{
    uloop_timeout_set(timeout, 1000); // 重新设置定时器，间隔 1000 毫秒
    blob_buf_init(&b, 0); // 初始化 blob 缓冲区
    blobmsg_add_string(&b, "message", "hello world"); // 添加字符串 "hello": "world" 到缓冲区
    ubus_notify(ctx, &hello_obj, "pub", b.head, -1); // 发送通知
}

int main()
{
    uloop_init(); // 初始化事件循环

    ctx = ubus_connect(NULL); // 连接到 ubus

    ubus_add_uloop(ctx); // 将 ubus 添加到事件循环

    // 设置文件描述符关闭时执行标志
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC); 

    ubus_add_object(ctx, &hello_obj); // 将 hello 对象添加到 ubus

    notify_timer.cb = notify_timer_cb; // 设置定时器回调函数
    uloop_timeout_set(&notify_timer, 1000); // 设置定时器，1 秒后触发

    uloop_run(); // 运行事件循环
    uloop_done(); // 清理事件循环
    return 0;
}
