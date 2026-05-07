#include <stdio.h>
#include <string.h>

// ===================== 固定参数 =====================
#define BAUD_RATE    9600    // 波特率
#define DEV_ADDR     1       // 设备地址
// ====================================================

// 读光照寄存器指令（固定不变）
unsigned char CMD_READ_LUX[8] = {0x01, 0x03, 0x00, 0x07, 0x00, 0x02, 0x75, 0xCA};

// ------------------- 你必须实现的3个串口函数 -------------------
void UART_Init(void) {
    // 初始化串口 9600 8N1
}

void UART_SendByte(unsigned char dat) {
    // 发送1字节
}

unsigned char UART_RecvByte(unsigned int timeout) {
    // 接收1字节，超时返回0
}
// -------------------------------------------------------------

// Modbus CRC校验（不用管）
unsigned short CRC16_Modbus(unsigned char *buf, int len) {
    unsigned short crc = 0xFFFF;
    int i, j;
    for (i = 0; i < len; i++) {
        crc ^= buf[i];
        for (j = 0; j < 8; j++) {
            if (crc & 1) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

// ===================== 读取光照度核心函数 =====================
// 返回值：光照度（单位 Lux）
// =====================================================================
unsigned long Get_Lux(void) {
    unsigned char buf[9];
    unsigned long lux;

    // 1. 发送读数据指令
    for (int i = 0; i < 8; i++) {
        UART_SendByte(CMD_READ_LUX[i]);
    }

    // 2. 接收9字节返回
    for (int i = 0; i < 9; i++) {
        buf[i] = UART_RecvByte(200);
        if (buf[i] == 0) return 0; // 超时
    }

    // 3. 校验格式是否正确
    if (buf[0] != 1 || buf[1] != 0x03 || buf[2] != 0x04) {
        return 0;
    }

    // 4. 校验CRC
    unsigned short crc = CRC16_Modbus(buf, 7);
    if ((buf[7] != (crc & 0xFF)) || (buf[8] != (crc >> 8))) {
        return 0;
    }

    // 5. 计算光照度（JXBS-3001-GZ 标准格式）
    lux = ((unsigned long)buf[3] << 24) |
          ((unsigned long)buf[4] << 16) |
          ((unsigned long)buf[5] << 8)  |
          buf[6];

    return lux;
}

// ===================== 主函数 =====================
int main(void) {
    unsigned long lux;

    UART_Init();  // 串口初始化 9600

    while (1) {
        lux = Get_Lux();  // 读取光照
        printf("光照度：%lu Lux\n", lux);
    }
}