#include <uci.h>
#include <string.h>
#include <stdio.h>

#include "cfg.h"
#include "../modbus_rtu.h"

int modbus_rtu_load(void)
{
    // 定义变量
    modbus_rtu_node_t *rtu_node;
    const char *str;
    struct uci_context *ctx;
    struct uci_package *pkg;
    struct uci_section *sec;
    struct uci_element *ele;
    struct uci_option *opt;
    int ret = 0;
    printf("---------------------------------\n");

    /* 判断配置文件是否存在 */
    if (access(MODBUS_RTU_CFG, F_OK) != 0)
    {
        printf("Not find config user_list \n");
        return -1;
    }

    /* 申请一个UCI上下文. */
    ctx = uci_alloc_context();
    if (uci_load(ctx, MODBUS_RTU_CFG, &pkg) != 0)
    {
        printf("uci_load():user_list failed\n");
        uci_free_context(ctx);
        return -1;
    }

    // 用ele临时指针遍历所有的sections节点
    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "modbus_rtu") != 0)
        {
            continue;
        }

        rtu_node = (modbus_rtu_node_t *)calloc(1, sizeof(modbus_rtu_node_t));
        if (rtu_node == NULL)
        {
            printf("rtu_node calloc failed !!!!!");
            continue;
        }
        memset(rtu_node, 0, sizeof(modbus_rtu_node_t));

        // 读取传感器配置项:串口号、从站地址、周期、传感器编码、功能码、寄存器起始地址、寄存器数量
        str = uci_lookup_option_string(ctx, sec, "serial");
        if (str != NULL)
        {
            printf("serial:%s \n", str);
            strcpy(rtu_node->serial, str);
        }
        else
        {
            printf("Not found serial in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "slave");
        if (str != NULL)
        {
            printf("slave:%s \n", str);
            rtu_node->slave = atoi(str);
        }
        else
        {
            printf("Not found sensor_name in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "period");
        if (str != NULL)
        {
            printf("period:%s \n", str);
            rtu_node->period = atoi(str);
        }
        else
        {
            printf("Not found period in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }
        rtu_node->timeout = 1;
        str = uci_lookup_option_string(ctx, sec, "sensor_code");
        if (str != NULL)
        {
            strcpy(rtu_node->sensor_code, str);
            printf("sensor_code: %s \n", rtu_node->sensor_code);
        }
        else
        {
            printf("Not found sensor_name in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "fun_code");
        if (str != NULL)
        {
            rtu_node->function_id = atoi(str);
            printf("fun_code: %d \n", rtu_node->function_id);
        }
        else
        {
            printf("Not found fun_code in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "reg_start");
        if (str != NULL)
        {
            rtu_node->reg_addr = atoi(str);
            printf("reg_addr: %d \n", rtu_node->reg_addr);
        }
        else
        {
            printf("Not found reg_addr in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "reg_num");
        if (str != NULL)
        {
            rtu_node->reg_num = atoi(str);
            printf("reg_num: %d \n", rtu_node->reg_num);
        }
        else
        {
            printf("Not found reg_num in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        list_add(&rtu_node->list, &g_modbus_rtu_list);
    }

end:
    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    return ret;
}

static int modbus_serial_load(void)
{
    modbus_serial_t *serial_t;
    const char *str;
    struct uci_context *ctx;
    struct uci_package *pkg;
    struct uci_section *sec;
    struct uci_element *ele;

    printf("***************************\n");

    /* 判断配置文件是否存在 */
    if (access(MODBUS_SERIAL_CFG, F_OK) != 0)
    {
        printf("Not find config %s \n", MODBUS_SERIAL_CFG);
        return -1;
    }

    /* 申请一个UCI上下文. */
    ctx = uci_alloc_context();
    if (uci_load(ctx, MODBUS_SERIAL_CFG, &pkg) != 0)
    {
        printf("uci_load():user_list failed\n");
        uci_free_context(ctx);
        return -1;
    }

    uci_foreach_element(&pkg->sections, ele)
    {
        sec = uci_to_section(ele);
        if (strcmp(sec->type, "serial") != 0)
        {
            continue;
        }
        str = uci_lookup_option_string(ctx, sec, "enable");
        if (str != NULL && strcmp(str, "1") == 0)
        {
            printf("enable:%s \n", str);
        }
        else
        {
            printf("serial disable ! \n");
            return -1;
        }

        serial_t = (modbus_serial_t *)calloc(1, sizeof(modbus_serial_t));
        if (serial_t == NULL)
        {
            printf("serial_t calloc failed !!!!!");
            continue;
        }
        memset(serial_t, 0, sizeof(modbus_serial_t));

        str = uci_lookup_option_string(ctx, sec, "busname");
        if (str != NULL)
        {
            strcpy(serial_t->name, str);
            printf("serial_t->name:%s \n", str);
        }

        str = uci_lookup_option_string(ctx, sec, "serial_type");
        if (str != NULL)
        {
            printf("serial_type:%s \n", str);
            serial_t->serial_type = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "speed");
        if (str != NULL)
        {
            printf("speed:%s \n", str);
            serial_t->speed = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "data_bits");
        if (str != NULL)
        {
            printf("data_bits:%s \n", str);
            serial_t->data_bits = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "stop_bits");
        if (str != NULL)
        {
            printf("stop_bits:%s \n", str);
            serial_t->stop_bits = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "check_bits");
        if (str != NULL)
        {
            printf("check_bits:%s \n", str);
            strcpy(serial_t->check_bits, str);
        }

        list_add(&serial_t->list, &g_modbus_serial_list);
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    if (list_empty(&g_modbus_serial_list))
    {
        printf("No serial port is available \n");
        return -1;
    }

    printf("***************************\n");

    return 0;
}

int cfg_init(void)
{
    int rc;

    rc = modbus_serial_load();
    if (rc < 0)
    {
        printf("modbus_serial_load  failed\n");
        return -1;
    }
    printf("modbus_serial_load config success\n");

    rc = modbus_rtu_load();
    if (rc < 0)
    {
        printf("modbus_rtu_load  failed\n");
        return -1;
    }
    printf("modbus_rtu_load config success\n");
    return 0;
}
