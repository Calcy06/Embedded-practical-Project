#include "read_volume/read_volume.h"

int read_volume()
{
    int regs = 0;
    uint16_t buf = 0;
    
    for (int i = 0; i < 2; i++)
    {
        if (strcmp(rtuctl[i].sensor_name, "SGBJQ-1") == 0)
        {
            regs = modbus_read_registers(rtuctl[i].ctx, 1, 1, &buf); 
            if (regs == -1) {
                printf("volume传感器读取失败\n");
            
                return -1;
            }
            break;
        }
    }
    
    return (int)buf;
}