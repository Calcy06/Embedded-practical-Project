#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>

#include "../modbus_rtu.h"

struct ubus_context *ubus_ctx = NULL;


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

static struct ubus_method modbus_methods[] = {
    {.name = "write", .handler = modbus_write_data}

};

static struct ubus_object_type modbus_obj_type = UBUS_OBJECT_TYPE("modbus_rtu", modbus_methods);

/** 定义ubus对象 **/
struct ubus_object modbus_rtu_object = {
    .name = "modbus_rtu",     /* object的名字 */
    .type = &modbus_obj_type, /* object类型 */
    .methods = modbus_methods,
    .n_methods = ARRAY_SIZE(modbus_methods),
};

static void ubus_add_fd(void)
{
    ubus_add_uloop(ubus_ctx);

#ifdef FD_CLOEXEC
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC);
#endif
}

void modbus_ubus_done(void)
{
    if (ubus_ctx)
        ubus_free(ubus_ctx);
}

int modbus_ubus_init(void)
{
    int ret;

    ubus_ctx = ubus_connect(NULL);
    if (!ubus_ctx)
    {
        printf("Failed to connect to ubus\n");
        return -1;
    }

    printf("connected as %08x\n", ubus_ctx->local_id);
    ubus_add_fd();

    ret = ubus_add_object(ubus_ctx, &modbus_rtu_object);
    if (ret)
    {
        printf("Failed to add object: %s\n", ubus_strerror(ret));
        return -1;
    }

    return 0;
}