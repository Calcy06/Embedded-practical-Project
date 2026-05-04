#include "write_register/write_register.h"

int write_register(int f)
{   
    if (f)
    {
        for (int i = 0; i < 2; i++)
        {
            if(strcmp(rtuctl[i].sensor_name, "SGBJQ-1") == 0)
            {
                modbus_write_register(rtuctl[i].ctx, 0x0001, 0x0001);
                printf("SGBJQ-1的从站地址：%d,", rtuctl[i].slave);
                printf("chengguo1\n");
            }
        }
        f = 0;
    }
    else
    {
        for (int i = 0; i < 2; i++)
        {
            if(strcmp(rtuctl[i].sensor_name, "SGBJQ-1") == 0)
            {
                modbus_write_register(rtuctl[i].ctx,0x0002, 0x0001);
                printf("SGBJQ-1的从站地址：%d,", rtuctl[i].slave);
                printf("chengguo1\n");
            }
        }
        f = 1;
    }

    return f;
}