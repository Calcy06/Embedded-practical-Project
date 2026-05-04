#include "read_uci.h"

char *name;
int serial_type;
int speed;
int data_bits;
int stop_bits;
char check_bits[2];
struct list_head sensor_list;

// 读取传感器配置
static void read_uci_sensor()
{
    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;
    struct uci_section *sec = NULL;

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作

    // 加载配置文件
    uci_load(ctx, PATH_SENSOR, &pkg);

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "sensor") == 0)
        {
            // 分配内存
            struct sensor_node *node = malloc(sizeof(*node));
            if (node == NULL)
            {
                printf("malloc failed\n");
                return;
            }

            // 读取数据
            const char *s = uci_lookup_option_string(ctx, sec, "slave");
            const char *n = uci_lookup_option_string(ctx, sec, "name");

            // 赋值
            node->slave = s ? atoi(s) : 0;
            node->name = n ? strdup(n) : NULL;

            // 添加到链表
            INIT_LIST_HEAD(&node->list);
            list_add_tail(&node->list, &sensor_list);
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

// 读取串口配置
static void read_uci_serial()
{
    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;
    struct uci_section *sec = NULL;

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作

    // 加载配置文件
    uci_load(ctx, PATH_SERIAL, &pkg);

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "serial") == 0)
        {
            // 读取数据
            const char *n = uci_lookup_option_string(ctx, sec, "name");
            const char *t = uci_lookup_option_string(ctx, sec, "serial_type");
            const char *s = uci_lookup_option_string(ctx, sec, "speed");
            const char *d = uci_lookup_option_string(ctx, sec, "data_bits");
            const char *st = uci_lookup_option_string(ctx, sec, "stop_bits");
            const char *c = uci_lookup_option_string(ctx, sec, "check_bits");

            // 赋值
            name = strdup(n ? n : "NoName");
            serial_type = t ? atoi(t) : 0;
            speed = s ? atoi(s) : 9600;
            data_bits = d ? atoi(d) : 8;
            stop_bits = st ? atoi(st) : 1;
            if (check_bits_str)
            {
                check_bits[0] = check_bits_str[0];
                check_bits[1] = '\0';
            }
            else
            {
                check_bits[0] = 'N';
                check_bits[1] = '\0';
            }
        }
        uci_unload(ctx, pkg);
        uci_free_context(ctx);
    }
}

    void read_uci()
    {
        // 初始化传感器链表
        INIT_LIST_HEAD(&sensor_list);
        // 读取传感器的从站地址和名字
        read_uci_sensor();
        // 读取总线数据
        read_uci_serial();
    }