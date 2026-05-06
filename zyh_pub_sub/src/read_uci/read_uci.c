#include "read_uci.h"

char *name;
int speed;
int data_bits;
int stop_bits;
char check_bits;
struct list_head sensor_list;

static void read_uci_sensor()
{
    INIT_LIST_HEAD(&sensor_list);

    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作
    if (!ctx)
    {
        printf("shibai\n");

        return;
    }
    //加载配置文件
    if (uci_load(ctx, CFG_SENSOR, &pkg) != UCI_OK) 
    {
        uci_free_context(ctx);
        printf("sensor\n");
        return;
    }

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        struct uci_section *sec = uci_to_section(ele);
        if (strcmp(sec->type, "sensor") == 0)
        {
            struct my_node *node = malloc(sizeof(*node));
            const char *s;            
            s = uci_lookup_option_string(ctx, sec, "slave");
            node->slave = s ? atoi(s) : 0;
            s = uci_lookup_option_string(ctx, sec, "name");
            node->name = s ? strdup(s) : strdup("noname");
            INIT_LIST_HEAD(&node->list);
            list_add_tail(&node->list, &sensor_list);
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
}

static void read_uci_serial()
{
    struct uci_context *ctx = NULL;
    struct uci_package *pkg = NULL;

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作

    //加载配置文件
    if (uci_load(ctx, CFG_SERIAL, &pkg) != UCI_OK) 
    {
        printf("serial\n");
        uci_free_context(ctx);
        return;
    }

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        struct uci_section *sec = uci_to_section(ele);
        if (strcmp(sec->type, "serial") == 0)
        {
            const char *s;            
            s = uci_lookup_option_string(ctx, sec, "name");
            name = s ? strdup(s) : strdup("noname");
            s = uci_lookup_option_string(ctx, sec, "speed");
            speed = s ? atoi(s) : 0;
            s = uci_lookup_option_string(ctx, sec, "data_bits");
            data_bits = s ? atoi(s) : 0;
            s = uci_lookup_option_string(ctx, sec, "stop_bits");
            stop_bits = s ? atoi(s) : 0;
            s = uci_lookup_option_string(ctx, sec, "check_bits");
            check_bits = s ? s[0] : '0';           
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