#include "modbus_init.h"

modbus_serial_t arr_sensor[SIZE];

// 定义一个函数，用于注册Modbus串口
int modbus_serial_registers()
{
    read_uci();
    if (!name)
    {
        printf("错误：串口名称未配置\n");
        return -1;
    }
   printf("串口配置: name=%s, speed=%d, check_bits=%c\n", 
           name, speed, check_bits); 

    for (int i = 0; i < 2; i++)
    {
        snprintf(arr_sensor[i].name, sizeof(arr_sensor[i].name), "%s", name);
        arr_sensor[i].speed = speed;
        arr_sensor[i].data_bits = data_bits;
        arr_sensor[i].stop_bits = stop_bits;
        arr_sensor[i].check_bits = check_bits;
        // 打开Modbus RTU端口
        // 参数1：串口设备路径
        // 参数2：波特率
        // 参数3：校验位（'N'、'E'、'O'）
        // 参数4：数据位
        // 参数5：停止位
        arr_sensor[i].ctx = modbus_new_rtu(arr_sensor[i].name, arr_sensor[i].speed, arr_sensor[i].check_bits, arr_sensor[i].data_bits, arr_sensor[i].stop_bits);
        if (arr_sensor[i].ctx == NULL)
        {
            printf("第%d个传感器的ctx失败\n", i);

            return -1;
        }

        // 建立modbus连接
        if (modbus_connect(arr_sensor[i].ctx) == -1)
        {
            printf("第%d个传感器的modbus连接失败\n", i);
            modbus_free(arr_sensor[i].ctx);
            return -1;
        }

        // 设置应答延时（可选）
        // 参数1：秒
        // 参数2：微秒
        modbus_set_response_timeout(arr_sensor[i].ctx, 3, 0);
    }

    struct sensor_node *pos;
    int i = 0;
    list_for_each_entry(pos, &sensor_list, list)
    {
        if (i >= 2)
        {
            break;
        }

        // 设置从设备地址
        modbus_set_slave(arr_sensor[i].ctx, pos->slave);
        // 设置传感器名称
        snprintf(arr_sensor[i].sensor_name, sizeof(arr_sensor[i].sensor_name), "%s", pos->name);
        // 把从设备地址存信息起来
        arr_sensor[i].slave = pos->slave;

        i++;
    }
    return 0;
}