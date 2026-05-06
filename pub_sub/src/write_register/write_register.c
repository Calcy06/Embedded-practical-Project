#include "write_register.h"

void write_register()
{
    for (int i = 0; i < 2; i++)
    {
        if (strcmp(arr_sensor[i].sensor_name, "SHENGGUANG") == 0)
        {
            int res=modbus_write_register(arr_sensor[i].ctx, 0x0001, 0x0001);
            if(res>0){
                printf("chenggong\n");
            }
            else if(res==-1)
            {
                printf("shibai\n");
                printf("失败原因：%s\n", modbus_strerror(errno));
            }    
            else
            {
                printf("qt\n");
            }    
            break;
        }
    }
}