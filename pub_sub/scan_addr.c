#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <modbus/modbus.h>

#define ADDR_MIN     1
#define ADDR_MAX     247
#define SCAN_TIMEOUT 200

// 支持的波特率（该传感器只支持这3个）
int baud_rates[] = {9600, 4800, 2400};
int baud_count = sizeof(baud_rates)/sizeof(int);

int try_scan(const char* port, int baud, int *found_addr)
{
    modbus_t *ctx = modbus_new_rtu(port, baud, 'N', 8, 1);
    if (!ctx) return -1;

    modbus_set_response_timeout(ctx, 0, SCAN_TIMEOUT * 1000);
    if (modbus_connect(ctx) == -1) {
        modbus_free(ctx);
        return -1;
    }

    for (int addr = ADDR_MIN; addr <= ADDR_MAX; addr++) {
        modbus_set_slave(ctx, addr);
        uint16_t reg;

        // 读地址寄存器 0x0100
        int rc = modbus_read_registers(ctx, 0x0100, 1, &reg);
        if (rc == 1) {
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

void scan_auto(const char* port)
{
    int found_baud = -1;
    int found_addr = -1;

    printf("自动扫描波特率 + 地址...\n");

    for (int i=0; i<baud_count; i++) {
        int baud = baud_rates[i];
        printf("正在测试波特率：%d...\n", baud);

        found_baud = try_scan(port, baud, &found_addr);
        if (found_baud != -1) break;
    }

    if (found_baud != -1) {
        printf("\n✅ 扫描成功！\n");
        printf("波特率：%d\n", found_baud);
        printf("地址：%d\n", found_addr);
    } else {
        printf("\n❌ 未找到传感器\n");
    }
}

int main(int argc, char *argv[])
{
    if (argc != 2) {
        printf("用法：%s <串口>\n", argv[0]);
        return -1;
    }
    scan_auto(argv[1]);
    return 0;
}