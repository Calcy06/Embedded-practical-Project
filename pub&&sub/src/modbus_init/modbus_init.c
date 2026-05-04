#include "modbus_init.h"

modbus_serial_t arr_sensor[SIZE];

// 定义一个函数，用于注册Modbus串口
int modbus_serial_registers(modbus_serial_t *arr_sensor)
{
    read_uci();
    if (!name)
    {
        printf("错误：串口名称未配置\n");
        return -1;
    }
    printf("串口配置: name=%s, type=%d, speed=%d, check_bits=%s\n",
           name, serial_type, speed, check_bits);

    for (int i = 0; i < 2; i++)
    {
        snprintf(arr_sensor[i].name, sizeof(arr_sensor[i].name), "%s", name);
        arr_sensor[i].serial_type = serial_type;
        arr_sensor[i].speed = speed;
        arr_sensor[i].data_bits = data_bits;
        arr_sensor[i].stop_bits = stop_bits;
        snprintf(arr_sensor[i].check_bits, sizeof(arr_sensor[i].check_bits), "%s", check_bits);
        // 打开Modbus RTU端口
        // 参数1：串口设备路径
        // 参数2：波特率
        // 参数3：校验位（'N'、'E'、'O'）
        // 参数4：数据位
        // 参数5：停止位
        arr_sensor[i].ctx = modbus_new_rtu(arr_sensor[i].name, arr_sensor[i].speed, (char)arr_sensor[i].check_bits[0], arr_sensor[i].data_bits, arr_sensor[i].stop_bits);
        if (arr_sensor[i].ctx == NULL)
        {
            printf("第%d个传感器的ctx失败\n", i);

            return -1;
        }

        // 设置Modbus调试模式
        modbus_set_debug(arr_sensor[i].ctx, TRUE);

        // 设置串口模式
        // 串口类型为1时，设置为RS232
        if (arr_sensor[i].serial_type == 1)
        {
            modbus_rtu_set_serial_mode(arr_sensor[i].ctx, MODBUS_RTU_RS232);
        }
        else // 否则，设置为RS485
        {
            modbus_rtu_set_serial_mode(arr_sensor[i].ctx, MODBUS_RTU_RS485);
        }

        // 设置RTS自动控制方向（可选）
        modbus_rtu_set_rts(arr_sensor[i].ctx, MODBUS_RTU_RTS_UP);

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

    struct serson_node *pos;
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