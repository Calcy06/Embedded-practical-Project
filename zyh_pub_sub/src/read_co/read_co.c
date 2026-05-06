#include "read_co/read_co.h"

void free_sensor_list()
{
    struct my_node *node, *tmp;

    list_for_each_entry_safe(node, tmp, &sensor_list, list) {
        free(node->name);  // 释放字符串
        list_del(&node->list); // 从链表删除
        free(node);        // 释放节点
    }
}

void free_serial_cfg()
{
    if (name)
        free(name);
}

int read_co()
{
    int regs = 0;
    uint16_t buf = 0;
    
    for (int i = 0; i < 2; i++)
    {
        if (strcmp(rtuctl[i].sensor_name, "CO") == 0)
        {
            regs = modbus_read_registers(rtuctl[i].ctx, 0, 1, &buf);
            printf("co的从站是：%d\n", rtuctl[i].slave);
            if (regs == -1) {
                printf("co传感器读取失败\n");
            
                return -1;
            }
            break;
        }
    }
    
    return (int)buf;
}