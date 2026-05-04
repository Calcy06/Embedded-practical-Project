#include <uci.h>
#include <unistd.h>
#include <string.h>

int main(int argc, char *argv[])
{
    struct uci_context *ctx;
    struct uci_package *pkg;
    struct uci_section *sec;
    const char *ip_str;
    const char *port_str;

    ctx = uci_alloc_context(); // 创建上下文，进行uci配置必要操作

    //加载配置文件
    uci_load(ctx, argv[1], &pkg);

    struct uci_element *ele; // 创建一个链表进行搜寻操作
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "client") == 0)
        {
            ip_str = uci_lookup_option_string(ctx, sec, "ip");
            LOG("SERVER_IP: %s\n", ip_str);

            port_str = uci_lookup_option_string(ctx, sec, "port");
            LOG("SERVER_PORT: %s\n", port_str);
        }
    }
    uci_unload(ctx, pkg);
    uci_free_context(ctx);
    return 0;
}