#include "ubus.h"
#include "modbus.h"
#include <libubus.h>
#include <libubox/blobmsg_json.h>
#include <string.h>

static struct ubus_context *g_ubus_ctx = NULL;
static struct ubus_event_handler g_event_handler;

// 事件订阅回调（收到ubus事件时触发）
static void ubus_event_cb(struct ubus_context *ctx, struct ubus_event_handler *ev,
                          const char *type, struct blob_attr *msg)
{
    struct blob_attr *cur;
    int rem;

    // 只处理我们关心的 "alarm.ctrl" 事件
    if (strcmp(type, "alarm.ctrl") != 0)
        return;

    // 解析事件数据
    blobmsg_for_each_attr(cur, msg, rem) {
        if (strcmp(blobmsg_name(cur), "play") == 0) {
            int enable = blobmsg_get_u32(cur);
            modbus_alarm_play(enable);
        }
    }
}

int ubus_alarm_init(void)
{
    // 连接ubus
    g_ubus_ctx = ubus_connect(NULL);
    if (!g_ubus_ctx)
        return -1;

    // 把ubus事件加入uloop循环
    ubus_add_uloop(g_ubus_ctx);

    // 初始化事件处理器
    memset(&g_event_handler, 0, sizeof(g_event_handler));
    g_event_handler.cb = ubus_event_cb;

    // 订阅 "alarm.ctrl" 事件
    ubus_register_event_handler(g_ubus_ctx, &g_event_handler, "alarm.ctrl");

    return 0;
}

// 发送状态到ubus事件总线
void ubus_alarm_publish(alarm_status_t *st)
{
    struct blob_buf b = {0};
    if (!g_ubus_ctx || !st)
        return;

    // 构建JSON数据
    blob_buf_init(&b, 0);
    blobmsg_add_u16(&b, "volume", st->volume);
    blobmsg_add_u16(&b, "status", st->status);
    blobmsg_add_u16(&b, "mode", st->mode);
    blobmsg_add_u16(&b, "track", st->track);

    // 发送事件（用ubus_send_event，不是ubus_notify！）
    ubus_send_event(g_ubus_ctx, "alarm.status", b.head);
    blob_buf_free(&b);
}