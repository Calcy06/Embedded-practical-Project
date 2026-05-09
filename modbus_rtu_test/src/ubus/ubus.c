#include <libubox/uloop.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>
#include "../modbus_rtu.h"

// 【自定义全局变量】UBus上下文
struct ubus_context *ubus_ctx = NULL;

// ====================== UBus写控制回调 ======================
// 【核心函数】ubus_notify/modbus_write_register
// 【自定义函数】UBus write接口处理函数：控制设备
static int modbus_write_data(struct ubus_context *ctx, struct ubus_object *obj,
                             struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    modbus_rtu_node_t *pos, *npos;
    // 遍历传感器，找到声光报警器并控制
    list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
    {
        if (strcmp(pos->sensor_code, "SGBJQ") == 0)
        {
            modbus_set_slave(pos->ctx, pos->slave); //设置从机地址
            modbus_write_register(pos->ctx, 1, 1); // 【第三方库】写寄存器，触发报警
            break;
        }
    }
    return 0;
}

// 【自定义】UBus方法集合：提供write接口
static struct ubus_method modbus_methods[] = {
    {.name = "write", .handler = modbus_write_data}
};

// 【第三方库】定义UBus对象类型
static struct ubus_object_type modbus_obj_type = UBUS_OBJECT_TYPE("modbus_rtu", modbus_methods);

// 【自定义】UBus服务对象（系统通过此对象通信）
struct ubus_object modbus_rtu_object = {
    .name = "modbus_rtu",
    .type = &modbus_obj_type,
    .methods = modbus_methods,
    .n_methods = ARRAY_SIZE(modbus_methods),
};

// ====================== UBus初始化 ======================
// 【自定义函数】初始化UBus连接+注册服务
int modbus_ubus_init(void)
{
    int ret;
    ubus_ctx = ubus_connect(NULL); // 【第三方库】连接UBus
    ubus_add_uloop(ubus_ctx);      // 【第三方库】绑定事件循环
    ret = ubus_add_object(ubus_ctx, &modbus_rtu_object); // 注册服务
    return 0;
}

// 【自定义函数】释放UBus资源
void modbus_ubus_done(void)
{
    if (ubus_ctx)
        ubus_free(ubus_ctx);
}