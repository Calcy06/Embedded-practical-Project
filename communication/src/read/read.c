#include "read.h"
#include <uci.h>
#include <stdio.h>

#define CFG_PATH "/root/test/com/cfg/client"
#define BUF_SIZE 64

char SERVER_IP[BUF_SIZE];
char SERVER_PORT[BUF_SIZE];

void read_uci()
{
    // 创建uci上下文
    struct uci_context *ctx = uci_alloc_context();
    struct uci_package *pkg;

    if (!ctx || uci_load(ctx, CFG_PATH, &pkg))
    {
        printf("加载UCI配置失败\n");
        if (ctx)
            uci_free_context(ctx);
        return;
    }

    struct uci_element *ele;
    uci_foreach_element(&pkg->sections, ele)
    {
        struct uci_section *sec = uci_to_section(ele);
        if (!strcmp(sec->type, "client"))
        {
            const char *ip = uci_lookup_option_string(ctx, sec, "ip");
            const char *port = uci_lookup_option_string(ctx, sec, "port");
            snprintf(SERVER_IP, BUF_SIZE, "%s", ip);
            snprintf(SERVER_PORT, BUF_SIZE, "%s", port);
        }
    }
    // 释放UCI资源
    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    printf("读取配置: IP=%s, PORT=%s\n", SERVER_IP, SERVER_PORT);
}
