#include <stdio.h>
#include <libubox/uloop.h>
#include "modbus.h"
#include "ubus.h"
#include "config.h"

static struct uloop_timeout g_poll_timer;

// 定时轮询+发布
static void poll_timer_cb(struct uloop_timeout *t)
{
    alarm_status_t st = {0};
    modbus_alarm_read(&st);
    ubus_alarm_publish(&st);
    uloop_timeout_set(t, MODBUS_POLL_INTERVAL);
}

int main(void)
{
    // 初始化Modbus
    if (modbus_alarm_init() < 0) {
        printf("modbus init failed\n");
        return -1;
    }

    // 初始化uloop
    uloop_init();

    // 初始化ubus
    if (ubus_alarm_init() < 0) {
        printf("ubus init failed\n");
        return -1;
    }

    // 启动定时轮询
    g_poll_timer.cb = poll_timer_cb;
    uloop_timeout_set(&g_poll_timer, 1000);

    printf("alarm_ubus running...\n");
    uloop_run();

    return 0;
}