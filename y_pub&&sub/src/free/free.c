void free_sensor_list()
{
    struct my_node *node, *tmp;

    list_for_each_entry_safe(node, tmp, &sensor_list, list)
    {
        free(node->name);      // 释放字符串
        list_del(&node->list); // 从链表删除
        free(node);            // 释放节点
    }
}

void free_serial_cfg()
{
    if (name)
        free(name);
}
