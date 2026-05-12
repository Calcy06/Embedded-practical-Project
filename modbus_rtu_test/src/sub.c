#include <stdio.h>
#include <fcntl.h>
#include <libubox/blobmsg_json.h>
#include <libubox/uloop.h>
#include <libubus.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <libubox/usock.h>
#include <json-c/json.h> //头文件要引入

#include "modbus_rtu.h"
#include "cfg/cfg.h"
#include "ubus/ubus.h"


// UBus 上下文 & 订阅
static struct ubus_context *ctx;
static uint32_t obj_id;         // modbus_rtu 对象ID
static struct ubus_subscriber sub_event;

// TCP 客户端 socket
struct uloop_fd c_fd;

// TCP 客户端初始化
int client_init(void)
{
    int type = USOCK_TCP | USOCK_NOCLOEXEC | USOCK_IPV4ONLY;
    c_fd.fd = usock(type, "192.168.4.94", "60000");

    if (c_fd.fd < 0)
    {
        printf("TCP 连接失败：%s\n", strerror(errno));
        return -1;
    }

    uloop_fd_add(&c_fd, ULOOP_READ);
    printf("TCP 连接成功 \n");
    return 0;
}

// 订阅回调：收到 modbus_rtu 事件 → 转JSON → 发给TCP服务器
static int subscriber_cb(struct ubus_context *ctx, struct ubus_object *obj,
                         struct ubus_request_data *req, const char *method,
                         struct blob_attr *msg)
{
    // 只处理我们关心的事件名
    if (strcmp(method, "modbus_rtu") != 0) {
        return 0;
    }

    // 格式化JSON字符串
    char *json_string = blobmsg_format_json(msg, true);
    printf("【收到 Modbus 数据】%s\n", json_string);

    // 转发给 TCP 服务器（带换行方便解析）
    write(c_fd.fd, json_string, strlen(json_string));
    write(c_fd.fd, "\n", 1);

    free(json_string);
    return 0;
}

// 定时器：每10秒调用 modbus_rtu 的 write 方法（控制声光报警器）
void timer_cb(struct uloop_timeout *timer)
{
    printf("定时器：调用 modbus_rtu write 方法 → 触发声光报警器\n");

    // 调用 ubus 方法：modbus_rtu -> write
    ubus_invoke(ctx, obj_id, "write", NULL, NULL, NULL, 3000);

    // 10秒后重复
    uloop_timeout_set(timer, 10000);
}

int main(void)
{
    // 1. 初始化 TCP 客户端
    if (client_init() != 0)
    {
        printf("TCP 初始化失败\n");
        return -1;
    }

    // 2. 初始化事件循环
    struct uloop_timeout t;
    t.cb = timer_cb;
    uloop_init();

    // 3. 连接 UBus
    ctx = ubus_connect(NULL);
    if (!ctx) {
        printf("UBus 连接失败\n");
        return -1;
    }
    ubus_add_uloop(ctx);

    // 4. 注册订阅者
    sub_event.cb = subscriber_cb;
    ubus_register_subscriber(ctx, &sub_event);

    // 5. 订阅 modbus_rtu 设备事件
    const char *object_modbus = "modbus_rtu";
    ubus_lookup_id(ctx, object_modbus, &obj_id);
    ubus_subscribe(ctx, &sub_event, obj_id);

    // 6. 启动 10 秒定时器
    uloop_timeout_set(&t, 10000);

    printf("=== Modbus UBUS 订阅服务已启动，等待数据... ===\n");
    uloop_run();
    uloop_done();

    return 0;
}