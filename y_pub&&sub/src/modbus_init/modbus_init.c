#include "modbus_init/modbus_init.h"

// 用modbus_serial_t结构体封装每个 Modbus 设备
modbus_serial_t arr_aensor[SIZE];

// 定义一个函数，用于注册Modbus串口
int modbus_serial_registers(modbus_serial_t *node)
{
    // 读取uci配置文件，获取串口参数
    read_uci();
    if (!name)
    {
        printf("错误：串口名称未配置\n");
        return -1;
    }
    printf("串口配置：name=%s, speed=%d, check_bits=%c\n", name, speed, check_bits);

    for (int i = 0; i < 2; i++)
    {
        snprintf(arr_aensor[i].name, sizeof(arr_aensor[i].name), "%s", name);
        arr_aensor[i].check_bits = check_bits;
        arr_aensor[i].speed = speed;
        arr_aensor[i].data_bits = data_bits;
        arr_aensor[i].stop_bits = stop_bits;
        // 打开Modbus RTU端口
        // 参数1：串口设备路径
        // 参数2：波特率
        // 参数3：校验位（'N'、'E'、'O'）
        // 参数4：数据位
        // 参数5：停止位
        arr_aensor[i].ctx = modbus_new_rtu(arr_aensor[i].name,
                                           arr_aensor[i].speed,
                                           arr_aensor[i].check_bits,
                                           arr_aensor[i].data_bits,
                                           arr_aensor[i].stop_bits);
        if (arr_aensor[i].ctx == NULL)
        {
            printf("%s的ctx失败\n", arr_aensor[i].name);
            return -1;
        }

        // 建立Modbus连接
        if (modbus_connect(arr_aensor[i].ctx) == -1)
        {
            printf("%s的modbus连接失败\n", arr_aensor[i].name);
            modbus_free(arr_aensor[i].ctx);

            return -1;
        }

        // 设置应答延时（可选）
        // 参数1：秒
        // 参数2：微秒
        modbus_set_response_timeout(arr_aensor[i].ctx, 3, 0);
        arr_aensor[i].sensor_name[0] = '\0';
    }
    struct my_node *pos;
    int i = 0;
    list_for_each_entry(pos, &sensor_list, list)
    {
        if (i >= 2)
            break;

        // 设置从站地址
        modbus_set_slave(arr_aensor[i].ctx, pos->slave);

        // 将传感器名称复制到modbus_serial_t结构体中
        snprintf(arr_aensor[i].sensor_name, sizeof(arr_aensor[i].sensor_name), "%s", pos->name);
        //保存从站地址到modbus_serial_t结构体中
        arr_aensor[i].slave = pos->slave;
        
        i++;
    }

    return 0;
}