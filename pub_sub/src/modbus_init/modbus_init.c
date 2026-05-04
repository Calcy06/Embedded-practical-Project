#include "modbus_init/modbus_init.h"

modbus_serial_t rtuctl[SIZE];

// 定义一个静态函数，用于注册Modbus串口
int modbus_serial_registers()
{
    read_uci();  
    if (!name) {
        printf("错误：串口名称未配置\n");
        return -1;
    }
    
    printf("串口配置：name=%s, speed=%d, check_bits=%c\n", 
           name, speed, check_bits);    
    for (int i = 0; i < 2; i++)
    {
        // snprintf(rtuctl[i].name, sizeof(rtuctl[i].name), "%s", name);
        strncpy(rtuctl[i].name, name, sizeof(rtuctl[i].name) - 1);
        rtuctl[i].name[sizeof(rtuctl[i].name) - 1] = '\0';

        rtuctl[i].check_bits = check_bits;
        rtuctl[i].speed = speed;
        rtuctl[i].data_bits = data_bits;
        rtuctl[i].stop_bits = stop_bits;
        rtuctl[i].ctx = modbus_new_rtu(rtuctl[i].name, rtuctl[i].speed, 
                                       rtuctl[i].check_bits, 
                                       rtuctl[i].data_bits, rtuctl[i].stop_bits);  
        if (rtuctl[i].ctx == NULL)
        {
            printf("%d的ctx失败\n", i);

            return -1;
        }     
        // 建立Modbus连接
        if (modbus_connect(rtuctl[i].ctx) == -1)
        {
            printf("%d的modbus连接失败\n", i);
            modbus_free(rtuctl[i].ctx);   

            return -1;                                           
        }  
        // 设置应答延时（可选）参数1：秒 参数2：微秒
        modbus_set_response_timeout(rtuctl[i].ctx, 3, 0);   
        rtuctl[i].sensor_name[0] = '\0';            
    }
    struct my_node *pos;
    int i = 0;
    list_for_each_entry(pos, &sensor_list, list) 
    {
        if (i >= 2)
        {
            break;
        }
        modbus_set_slave(rtuctl[i].ctx, pos->slave);
        snprintf(rtuctl[i].sensor_name, sizeof(rtuctl[i].sensor_name), "%s", pos->name);
        rtuctl[i].slave = pos->slave;
        i++;
    }

    return 0; 
}