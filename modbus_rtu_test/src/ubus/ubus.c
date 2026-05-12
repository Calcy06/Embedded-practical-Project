#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>

#include "../modbus_rtu.h"

struct ubus_context *ubus_ctx = NULL;

// 定义ubus方法，SGBJQ写控制回调
static int modbus_write_data(struct ubus_context *ctx, struct ubus_object *obj,
                             struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    modbus_rtu_node_t *pos, *npos;
    list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
    {
        if (strcmp(pos->sensor_code, "SGBJQ") == 0)
        {
            printf("control sgbjq...............\n");
            modbus_set_slave(pos->ctx, pos->slave);
            modbus_write_register(pos->ctx, 1, 1);
            break;
        }
    }

    return 0;
}

// 定义ubus方法合集
static struct ubus_method modbus_methods[] = {
    {.name = "write", .handler = modbus_write_data}

};

// 定义ubus对象类型
static struct ubus_object_type modbus_obj_type = UBUS_OBJECT_TYPE("modbus_rtu", modbus_methods);

/** 定义ubus对象 **/
struct ubus_object modbus_rtu_object = {
    .name = "modbus_rtu",     /* object的名字 */
    .type = &modbus_obj_type, /* object类型 */
    .methods = modbus_methods,
    .n_methods = ARRAY_SIZE(modbus_methods),
};

// 初始化 ubus 通信 + 设置文件描述符安全标志
static void ubus_add_fd(void)
{
    ubus_add_uloop(ubus_ctx);

#ifdef FD_CLOEXEC
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC);
#endif
}

//释放 ubus 资源
void modbus_ubus_done(void)
{
    if (ubus_ctx)
        ubus_free(ubus_ctx);
}

// 初始化UBus连接+注册服务
int modbus_ubus_init(void)
{
    int ret;

    // 连接到ubus
    ubus_ctx = ubus_connect(NULL);
    if (!ubus_ctx)
    {
        printf("Failed to connect to ubus\n");
        return -1;
    }

    printf("connected as %08x\n", ubus_ctx->local_id);
    ubus_add_fd();

    // 注册ubus对象
    ret = ubus_add_object(ubus_ctx, &modbus_rtu_object);
    if (ret)
    {
        printf("Failed to add object: %s\n", ubus_strerror(ret));
        return -1;
    }

    return 0;
}