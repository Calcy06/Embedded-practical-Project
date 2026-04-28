#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <modbus/modbus.h>
#include <libubox/uloop.h>

#define SERIAL_PORT "/dev/ttyUSB0"
#define BAUD_RATE 9600
#define SLAVE_ADDR 1
#define ALARM_REG_ADDR 0x0001 // 灯光寄存器

modbus_t *ctx;

// uloop 定时器结构体
static struct uloop_timeout g_alarm_timer;

// 初始化Modbus连接
int modbus_alarm_init(void)
{
    ctx = modbus_new_rtu(SERIAL_PORT, BAUD_RATE, 'N', 8, 1);
    if (!ctx) {
        perror("modbus_new_rtu failed");
        return -1;
    }

    modbus_set_slave(ctx, SLAVE_ADDR);
    modbus_set_response_timeout(ctx, 5, 0);  // 防超时
    if (modbus_connect(ctx) == -1) {
        perror("modbus_connect failed");
        modbus_free(ctx);
        return -1;
    }
    printf("Modbus 连接成功，设备地址：%d\n", SLAVE_ADDR);
    return 0;
}

// 控制报警器：1=亮灯，0=熄灯
void alarm_control(int enable)
{
    int ret;

    if (enable) {
        // 亮灯
        ret = modbus_write_register(ctx, ALARM_REG_ADDR, 0x0001);
    } else {
        // 熄灯
        ret = modbus_write_register(ctx, ALARM_REG_ADDR, 0x0000);
    }

    if (ret == -1) {
        perror("亮灯控制失败");
        return;
    }
    printf("灯光状态：%s\n", enable ? "亮灯" : "熄灯");
}

// 定时器回调函数：每秒翻转灯光状态
static void alarm_timer_cb(struct uloop_timeout *t)
{
    static int state = 0;
    state = !state;

    alarm_control(state);

    uloop_timeout_set(t, 1000);
}

int main()
{
    if (modbus_alarm_init() < 0) {
        return -1;
    }

    uloop_init();
    memset(&g_alarm_timer, 0, sizeof(g_alarm_timer));
    g_alarm_timer.cb = alarm_timer_cb;

    uloop_timeout_set(&g_alarm_timer, 1000);

    printf("uloop 灯光闪烁程序已启动，无限循环中...\n");

    uloop_run();

    uloop_done();
    alarm_control(0);
    modbus_close(ctx);
    modbus_free(ctx);

    return 0;
}