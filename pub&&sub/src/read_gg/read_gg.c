#include "read_gg.h"

int read_gg()
{
    int regs = 0;           // 读取到的寄存器数量
    int i = 0;              // 循环计数器
    uint16_t buf1[50] = {0}; // 存储读取寄存器数据的缓冲区，初始化为0

    for (i = 0; i < 2; i++)
    {
        if (strcmp(rtuctl[i].sensor_name, "GUANGGAN") == 0)
        {
            // 读取Modbus寄存器
            // 参数1：Modbus上下文（ctx）
            // 参数2：起始寄存器地址（0x01）
            // 参数3：读取寄存器的数量（0x01）
            // 参数4：用于存储读取数据的缓冲区（buf1）
            regs = modbus_read_registers(rtuctl[i].ctx, 0x01, 0x01, buf1);
            if (regs == -1)
            {
                printf("GUANGGAN传感器读取失败\n");
                return -1;
            }

            return (int)buf1[0]; // 返回读取到的第一个寄存器的值
        }
    }

    printf("没有找到GUANGGAN传感器\n");
    return -1; 
}