#include <libubox/uloop.h>
#include <libubox/ustream.h>
#include <libubox/utils.h>
#include <libubus.h>
#include <json-c/json.h>
#include <libubox/blobmsg_json.h>
#include <libdebug/libdebug.h>

#include "modbus_deal/modbus_deal.h"
#include "../modbus_rtu.h"

static dbg_md_t g_ubus_id = 0;
/* 创建 ubus debug */

#define UBUS_LOG(fmt, arg...)                                                   \
    do                                                                          \
    {                                                                           \
        dbg_logfile(g_ubus_id, "[%s %d]: " fmt, __FUNCTION__, __LINE__, ##arg); \
    } while (0)

#define UBUS_DEBUG(fmt, arg...)                          \
    do                                                   \
    {                                                    \
        dbg_printf(g_ubus_id, DBG_LV_DEBUG, fmt, ##arg); \
    } while (0)

#define UBUS_WARNING(fmt, arg...)                                                                     \
    do                                                                                                \
    {                                                                                                 \
        dbg_printf(g_ubus_id, DBG_LV_WARNING, "WARNING in %s [%d]: " fmt, __FILE__, __LINE__, ##arg); \
    } while (0)

#define UBUS_ERROR(fmt, arg...)                                                                   \
    do                                                                                            \
    {                                                                                             \
        dbg_printf(g_ubus_id, DBG_LV_ERROR, "ERROR in %s [%d]: " fmt, __FILE__, __LINE__, ##arg); \
    } while (0)

struct ubus_context *ubus_ctx = NULL;

static void test_client_subscribe_cb(struct ubus_context *ctx, struct ubus_object *obj)
{
    UBUS_LOG("Subscribers active: %d\n", obj->has_subscribers);
}

static int modbus_write_data(struct ubus_context *ctx, struct ubus_object *obj,
                             struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
    if (!msg || blob_len(msg) == 0) {
        printf("Invalid message\n");
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    char *str = blobmsg_format_json(msg, true);  
    UBUS_LOG("--------write cmd-----------------\n");
    UBUS_LOG("%s\n", str);  
    int ret = modbus_cmd_parse((unsigned char *)str);
    if(ret <0)
    {
        UBUS_LOG("Write command failure(UBUS_STATUS_INVALID_ARGUMENT)\n");  
        return UBUS_STATUS_INVALID_ARGUMENT;
    }
    free(str);
    UBUS_LOG("--------write cmd over-----------------\n");

    return 0;
}

/* 这个空的method列表，只是为了让object有个名字，这样client可以通过object name来订阅。 */
static struct ubus_method modbus_methods[] = {
    {.name = "set", .handler = modbus_set_period},
    {.name = "write", .handler = modbus_write_data}

};

static struct ubus_object_type modbus_obj_type = UBUS_OBJECT_TYPE("modbus_rtu", modbus_methods);

/** 定义ubus对象 **/
struct ubus_object modbus_rtu_object = {
    .name = "modbus_rtu",     /* object的名字 */
    .type = &modbus_obj_type, /* object类型 */
    .methods = modbus_methods,
    .n_methods = ARRAY_SIZE(modbus_methods),
    .subscribe_cb = test_client_subscribe_cb, /* 订阅回调函数：当收到新client的订阅时，会调用该函数 */
};

static void ubus_add_fd(void)
{
    ubus_add_uloop(ubus_ctx);

#ifdef FD_CLOEXEC
    fcntl(ctx->sock.fd, F_SETFD, fcntl(ctx->sock.fd, F_GETFD) | FD_CLOEXEC);
#endif
}

static void ubus_reconn_timer(struct uloop_timeout *timeout)
{
    static struct uloop_timeout retry =
        {
            .cb = ubus_reconn_timer,
        };
    int t = 2;
    UBUS_LOG("ubus_reconnect ..... \n");
    if (ubus_reconnect(ubus_ctx, NULL) != 0)
    {
        uloop_timeout_set(&retry, t * 1000);
        return;
    }

    ubus_add_fd();
}

static void ubus_connection_lost(struct ubus_context *ctx)
{
    ubus_reconn_timer(NULL);
}

void modbus_ubus_done(void)
{
    if (ubus_ctx)
        ubus_free(ubus_ctx);
}

int modbus_ubus_init(void)
{
    int ret;
    g_ubus_id = dbg_module_reg("UBUS");
    if (g_ubus_id < 0)
    {
        UBUS_ERROR("Register cfg module failed\n");
        return -1;
    }
    UBUS_LOG("debug module reg [UBUS] success\n");

    ubus_ctx = ubus_connect(NULL);
    if (!ubus_ctx)
    {
        UBUS_ERROR("Failed to connect to ubus\n");
        return -1;
    }

    UBUS_LOG("connected as %08x\n", ubus_ctx->local_id);
    ubus_ctx->connection_lost = ubus_connection_lost;
    ubus_add_fd();

    ret = ubus_add_object(ubus_ctx, &modbus_rtu_object);
    if (ret)
    {
        UBUS_ERROR("Failed to add object: %s\n", ubus_strerror(ret));
        return -1;
    }

    return 0;
}