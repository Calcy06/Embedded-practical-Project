#include "read_gg/read_gg.h"

int read_gg()
{
    int regs = 0;     // 读取到的寄存器数量
    uint16_t buf = 0; // 存储读取寄存器数据的缓冲区，初始化为0

    for (int i = 0; i < 2; i++)
    {
        if (strcmp(rtuctl[i].sensor_name, "gg") == 0)
        {
            // 读取Modbus寄存器
            // 参数1：Modbus上下文（ctx）
            // 参数2：起始寄存器地址（0x01）
            // 参数3：读取寄存器的数量（0x03）
            // 参数4：用于存储读取数据的缓冲区（buf）
            regs = modbus_read_registers(rtuctl[i].ctx, 0, 1, &buf);
            if (regs == -1)
            {
                printf("gg传感器读取失败\n");
                return -1;
            }
            break;
        }
    }

    return (int)buf;
}