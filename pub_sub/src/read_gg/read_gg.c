#include "read_gg.h"

int read_gg()
{
    int regs = 0;           // 读取到的寄存器数量
    int i = 0;              // 循环计数器
    uint16_t buf = 0; // 存储读取寄存器数据的缓冲区，初始化为0

    for (i = 0; i < 2; i++)
    {
        if (strcmp(arr_sensor[i].sensor_name, "GUANGGAN") == 0)
        {
            // 读取Modbus寄存器
            // 参数1：Modbus上下文（ctx）
            // 参数2：起始寄存器地址（0x01）
            // 参数3：读取寄存器的数量（0x01）
            // 参数4：用于存储读取数据的缓冲区（buf1）
            regs = modbus_read_registers(arr_sensor[i].ctx, 0x0007, 2, &buf);
            printf("gg的从站是: %d\n", arr_sensor[i].slave);
            if (regs == -1)
            {
                printf("GUANGGAN传感器读取失败\n");
                return -1;
            }
            else
            {
                printf("GUANGGAN传感器读取成功\n");
            }

            return (int)buf ; // 返回读取到的第一个寄存器的值
        }
    }

    printf("没有找到GUANGGAN传感器\n");
    return -1; 
}