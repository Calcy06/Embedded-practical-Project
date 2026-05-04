#include "write_register.h"

int write_register(int state)
{
    if(state)
    {
        for(int i = 0; i < 2; i++)
        {
            if(strcmp(arr_sensor[i].sensor_name, "SHENGGUANG") == 0)
            {
                modbus_write_register(arr_sensor[i].ctx, 0x001, 0x001);
                printf("亮灯\n");
            }
        }
        state = 0;
    }
    else
    {
        for(int i = 0; i < 2; i++)
        {
            if(strcmp(arr_sensor[i].sensor_name, "SHENGGUANG") == 0)
            {
                modbus_write_register(arr_sensor[i].ctx, 0x002, 0x001);
                printf("灭灯\n");
            }
        }
        state = 1;
    }
    return state;   
}