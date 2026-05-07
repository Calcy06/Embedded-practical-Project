#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <modbus/modbus.h>

// 地址：1~247 全扫
#define ADDR_MIN     1
#define ADDR_MAX     247

// 全部常用波特率
int baud_list[] = {1200,2400,4800,9600,19200,38400,57600,115200};
int baud_num = sizeof(baud_list)/sizeof(int);

#define TIMEOUT_MS 300

// 扫描：读光照寄存器 0x0007（你传感器必响应的地址）
int try_baud(const char *port, int baud, int *found_addr)
{
    modbus_t *ctx = modbus_new_rtu(port, baud, 'N', 8, 1);
    if (!ctx) return -1;

    modbus_set_response_timeout(ctx, 0, TIMEOUT_MS * 1000);
    if (modbus_connect(ctx) == -1) {
        modbus_free(ctx);
        return -1;
    }

    for (int addr = ADDR_MIN; addr <= ADDR_MAX; addr++) {
        modbus_set_slave(ctx, addr);
        uint16_t buf[2];

        // 读光照寄存器 0x0007，长度2（你手册规定！必响应）
        int rc = modbus_read_registers(ctx, 0x0007, 2, buf);
        if (rc == 2) {
            *found_addr = addr;
            modbus_close(ctx);
            modbus_free(ctx);
            return baud;
        }
    }

    modbus_close(ctx);
    modbus_free(ctx);
    return -1;
}

void scan_all(const char *port)
{
    int baud_ok = -1;
    int addr_ok = -1;

    printf("=== 光照传感器终极扫描 1~247地址 ===\n");

    for (int i=0; i<baud_num; i++) {
        int baud = baud_list[i];
        printf("波特率 %-6d 扫描中...", baud);

        baud_ok = try_baud(port, baud, &addr_ok);
        if (baud_ok != -1) {
            printf(" ✅ 找到！\n");
            break;
        } else {
            printf(" 无设备\n");
        }
    }

    if (baud_ok != -1) {
        printf("\n🎉 成功找到传感器！\n");
        printf("波特率：%d\n", baud_ok);
        printf("设备地址：%d\n", addr_ok);
    } else {
        printf("\n❌ 未找到，请 100%% 按下面检查接线！\n");
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        printf("用法：%s 串口名\n", argv[0]);
        printf("例：sudo %s /dev/ttyUSB0\n", argv[0]);
        return -1;
    }
    scan_all(argv[1]);
    return 0;
}