#include "read_sg.h"

int read_sg()
{
    int regs = 0;           // 读取到的寄存器数量
    int i = 0;              // 循环计数器
    uint16_t buf2[50] = {0}; // 存储读取寄存器数据的缓冲区，初始化为0
    
    for (i = 0; i < 2; i++)
    {
        if (strcmp(rtuctl[i].sensor_name, "SHENGGUANG") == 0)
        {
            regs = modbus_read_registers(rtuctl[i].ctx, 1, 1, buf2); 
            if (regs == -1) {
                printf("SHENGGUANG传感器读取失败\n");
            
                return -1;
            }
            return (int)buf2[0]; // 返回读取到的第一个寄存器的值
        }
    }
    
    printf("没有找到SHENGGUANG传感器\n");
    return -1; 
}