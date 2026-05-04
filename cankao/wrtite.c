#include <uci.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

#define LOG(fmt, ...)               \
    printf("%s %d:   ",             \
           __FUNCTION__, __LINE__); \
    printf(fmt, ##__VA_ARGS__)

int main(int argc, char *argv[])
{
    struct uci_context *ctx;
    struct uci_ptr ptr;
    char *str = strdup(argv[1]);

    ctx = uci_alloc_context();
    uci_lookup_ptr(ctx, &ptr, str, true);
    uci_set(ctx, &ptr);
    uci_commit(ctx, &ptr.p, false);

    uci_free_context(ctx);
    free(str);
    return (0);
}