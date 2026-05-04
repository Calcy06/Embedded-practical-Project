#include "read_uci.h"

char *name;
int speed;
int data_bits;
int stop_bits;
char check_bits;
struct list_head sensor_list;

// 读取uci配置文件，获取传感器的从站地址和名字，并存储在链表中
static void read_uci_sensor()
{
    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;
    struct uci_section *sec = NULL;

    INIT_LIST_HEAD(&sensor_list);

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作
    if (!ctx)
    {
        printf("uci_alloc_context:failed\n");
        return;
    }

    // 加载配置文件
    if (uci_load(ctx, CFG_SENSOR, &pkg) != UCI_OK)
    {
        printf("uci_load_sensor: failed\n");
        uci_free_context(ctx);
        return;
    }

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "sensor") == 0)
        {
            // 1. 分配节点
            struct my_node *node = malloc(sizeof(*node));
            if (!node)
                continue; // 防崩溃

            // 2. 读取配置
            const char *s = uci_lookup_option_string(ctx, sec, "slave");
            const char *n = uci_lookup_option_string(ctx, sec, "name");

            // 3. 赋值
            node->slave = s ? atoi(s) : 0;
            node->name = n ? strdup(n) : strdup("noname");

            // 4. 加入链表
            INIT_LIST_HEAD(&node->list);
            list_add_tail(&node->list, &sensor_list);
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

// 读取uci配置文件，获取总线数据，并打印出来
static void read_uci_serial()
{
    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;
    struct uci_section *sec = NULL;

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作
    if (!ctx)
    {
        printf("uci_alloc_context:failed\n");
        return;
    }

    if (uci_load(ctx, CFG_SERIAL, &pkg) != UCI_OK)
    {
        printf("uci_load_serial:failed\n");
        uci_free_context(ctx);
        return;
    }

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "serial") == 0)
        {
            // 1. 读取配置
            const char *n = uci_lookup_option_string(ctx, sec, "name");
            const char *s = uci_lookup_option_string(ctx, sec, "speed");
            const char *data_bits_str = uci_lookup_option_string(ctx, sec, "data_bits");
            const char *stop_bits_str = uci_lookup_option_string(ctx, sec, "stop_bits");
            const char *check_bits_str = uci_lookup_option_string(ctx, sec, "check_bits");

            // 2. 赋值
            name = strdup(n ? n : "NoName");
            speed = s ? atoi(s) : 4800;
            data_bits = data_bits_str ? atoi(data_bits_str) : 8;
            stop_bits = stop_bits_str ? atoi(stop_bits_str) : 1;
            check_bits = check_bits_str ? check_bits_str[0] : 'N';

            // // 3. 打印ubus总线配置
            // printf("name: %s\n", n ? n : "NoName");
            // printf("speed: %s\n", s ? s : "0");
            // printf("data_bits: %s\n", data_bits_str ? data_bits_str : "0");
            // printf("stop_bits: %s\n", stop_bits_str ? stop_bits_str : "0");
            // printf("check_bits: %s\n", check_bits_str ? check_bits_str : "0");
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

void read_uci()
{
    // 读取传感器的从站地址和名字
    read_uci_sensor();
    // 读取总线数据
    read_uci_serial();
}