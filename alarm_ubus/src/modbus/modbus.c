#include "modbus.h"
#include "config.h"
// 按你要求修正：modbus头文件路径
#include <modbus/modbus.h>
#include <stdio.h>
#include <unistd.h>

static modbus_t *g_modbus_ctx = NULL;

int modbus_alarm_init(void)
{
    g_modbus_ctx = modbus_new_rtu(MODBUS_SERIAL_DEV, MODBUS_BAUD, 'N', 8, 1);
    if (!g_modbus_ctx) return -1;

    modbus_set_slave(g_modbus_ctx, MODBUS_SLAVE_ADDR);
    if (modbus_connect(g_modbus_ctx) == -1) {
        modbus_free(g_modbus_ctx);
        g_modbus_ctx = NULL;
        return -1;
    }
    return 0;
}

void modbus_alarm_read(alarm_status_t *st)
{
    uint16_t buf[4] = {0};
    if (!g_modbus_ctx || !st) return;

    // 读4个保持寄存器：音量/模式/状态/曲目
    modbus_read_registers(g_modbus_ctx, 0x0001, 4, buf);

    st->volume = buf[0];
    st->mode   = buf[1];
    st->status = buf[2];
    st->track  = buf[3];
}

void modbus_alarm_play(int on)
{
    if (!g_modbus_ctx) return;
    if (on)
        modbus_write_register(g_modbus_ctx, 0x0001, 1);  // 播放
    else
        modbus_write_register(g_modbus_ctx, 0x0003, 1);  // 停止
}