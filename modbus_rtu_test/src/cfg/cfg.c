#include <uci.h>         // 【第三方库】UCI配置库
#include <string.h>
#include <stdio.h>
#include "cfg.h"
#include "../modbus_rtu.h"

// ====================== 加载传感器配置 ======================
// 【自定义函数】读取modbus_rtu配置，创建传感器节点
int modbus_rtu_load(void)
{
    modbus_rtu_node_t *rtu_node;
    struct uci_context *ctx;  // 【第三方库】UCI上下文
    struct uci_package *pkg;  // 【第三方库】配置包

    ctx = uci_alloc_context(); // 【第三方库】创建UCI上下文
    uci_load(ctx, MODBUS_RTU_CFG, &pkg); // 【第三方库】加载配置文件

    // 遍历配置节点
    uci_foreach_element(&pkg->sections, ele) //(问题)没有定义ele，应该是struct uci_element *ele;游标指针
    {
        sec = uci_to_section(ele);//(问题)没有定义sec，应该是struct uci_section *sec;
        rtu_node = calloc(1, sizeof(modbus_rtu_node_t)); // 【系统内置】申请1个modbus_rtu_node_t大小的内存
        // 读取配置：串口、从机号、周期、功能码、寄存器地址
        str = uci_lookup_option_string(ctx, sec, "serial");//问题没有定义str，应该是const char *str;
        //赋值
        strcpy(rtu_node->serial, str);
        // ... 省略其他配置读取 ...
        
        list_add(&rtu_node->list, &g_modbus_rtu_list); // 加入传感器链表
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
    return 0;
}

// ====================== 加载串口配置 ======================
// 【自定义函数】读取serial配置，创建串口节点
static int modbus_serial_load(void)
{
    modbus_serial_t *serial_t;
    // 逻辑同上：加载串口配置→创建结构体→加入串口链表
}

// ====================== 配置总初始化 ======================
// 【自定义函数】先加载串口，再加载传感器
int cfg_init(void)
{
    int rc;
    rc = modbus_serial_load();
    rc = modbus_rtu_load();
    return 0;
}