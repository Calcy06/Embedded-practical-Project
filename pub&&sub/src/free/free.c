#include "free.h"

// 统一释放所有资源
void free_all_resources(void)
{
    struct my_node *node, *tmp;

    // 1. 释放传感器链表
    list_for_each_entry_safe(node, tmp, &sensor_list, list) {
        free(node->name);      // 释放节点里的名字
        list_del(&node->list); // 从链表删除
        free(node);            // 释放节点本身
    }

    // 2. 释放 Modbus 串口资源
    for (int i = 0; i < 2; i++) {
        if (arr_sensor[i].ctx != NULL) {
            modbus_close(arr_sensor[i].ctx);   // 关闭串口
            modbus_free(arr_sensor[i].ctx);    // 释放Modbus上下文
            arr_sensor[i].ctx = NULL;          // 指针置空，防止野指针
        }
    }

    // 3. 释放全局配置 name
    if (name) {
        free(name);
        name = NULL;
    }
}