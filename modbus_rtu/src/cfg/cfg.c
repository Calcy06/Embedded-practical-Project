#include <uci.h>
#include <libdebug/libdebug.h>
#include <string.h>
#include <stdio.h>

#include "cfg.h"
#include "../modbus_rtu.h"

#define SN_FILE "/product_info/sn"
#define LIST_SEPARATOR "."

/* 定义debug�?*/
#define CFG_LOG(fmt, arg...)                                                   \
    do                                                                         \
    {                                                                          \
        dbg_logfile(g_cfg_id, "[%s %d]: " fmt, __FUNCTION__, __LINE__, ##arg); \
    } while (0)

#define CFG_DEBUG(fmt, arg...)                          \
    do                                                  \
    {                                                   \
        dbg_printf(g_cfg_id, DBG_LV_DEBUG, fmt, ##arg); \
    } while (0)

#define CFG_WARNING(fmt, arg...)                                                                     \
    do                                                                                               \
    {                                                                                                \
        dbg_printf(g_cfg_id, DBG_LV_WARNING, "WARNING in %s [%d]: " fmt, __FILE__, __LINE__, ##arg); \
    } while (0)

#define CFG_ERROR(fmt, arg...)                                                                   \
    do                                                                                           \
    {                                                                                            \
        dbg_printf(g_cfg_id, DBG_LV_ERROR, "ERROR in %s [%d]: " fmt, __FILE__, __LINE__, ##arg); \
    } while (0)

static int g_cfg_id = -1;

static int read_file(char *file, char *buffer)
{
    FILE *pf = fopen(file, "r");
    int len = 0;
    if (pf == NULL)
    {
        CFG_LOG("open %s failed\n", SN_FILE);
        return 0;
    }

    while (!feof(pf)) // 读文件
    {
        buffer[len] = fgetc(pf);
        // printf("buffer[%d] %02x\n",len,buffer[len]);
        if (buffer[len] == 0x0A)
        {
            buffer[len] = 0;

            break;
        }
        len++;
    }

    // if(fgets(buffer,sizeof(buffer)-1,pf) < 0)
    // {
    //     CFG_LOG("read sn error\n");
    //     return 1;
    // }

    fclose(pf);
    pf = NULL;
    return len;
}

int modbus_rtu_load(void)
{
    modbus_rtu_node_t *rtu_node;
    const char *str;
    struct uci_context *ctx;
    struct uci_package *pkg;
    struct uci_section *sec;
    struct uci_element *ele;
    struct uci_option *opt;
    int ret = 0;
    int i = 0, j = 0;
    CFG_LOG("---------------------------------\n");

    /* 判断配置文件是否存在 */
    if (access(MODBUS_RTU_CFG, F_OK) != 0)
    {
        CFG_ERROR("Not find config user_list \n");
        return -1;
    }

    /* 申请一个UCI上下文. */
    ctx = uci_alloc_context();
    if (uci_load(ctx, MODBUS_RTU_CFG, &pkg) != 0)
    {
        CFG_ERROR("uci_load():user_list failed\n");
        uci_free_context(ctx);
        return -1;
    }

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
            CFG_ERROR("rtu_node calloc failed !!!!!");
            continue;
        }
        memset(rtu_node, 0, sizeof(modbus_rtu_node_t));

        str = uci_lookup_option_string(ctx, sec, "r_enable");
        if (str != NULL)
        {
            if (strcmp(str, SENSOR_ENBLE) == 0)
            {
                rtu_node->r_enable = true;
            }
            else
            {
                rtu_node->r_enable = false;
            }
            CFG_LOG("rtu_node->r_enable:%d \n", rtu_node->r_enable);
        }
        else
        {
            CFG_ERROR("Not found r_enable in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "serial");
        if (str != NULL)
        {
            CFG_LOG("serial:%s \n", str);
            strcpy(rtu_node->serial, str);
        }
        else
        {
            CFG_ERROR("Not found serial in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "slave");
        if (str != NULL)
        {
            CFG_LOG("slave:%s \n", str);
            rtu_node->slave = atoi(str);
        }
        else
        {
            CFG_ERROR("Not found sensor_name in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        str = uci_lookup_option_string(ctx, sec, "period");
        if (str != NULL)
        {
            CFG_LOG("period:%s \n", str);
            rtu_node->period = atoi(str);
            rtu_node->tempperiod = atoi(str);
        }
        else
        {
            CFG_ERROR("Not found period in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }
        rtu_node->timeout = 1;
        str = uci_lookup_option_string(ctx, sec, "sensor_code");
        if (str != NULL)
        {
            //char buffer[20] = {0};

            // if (read_file(SN_FILE, buffer))
            // {
            //     sprintf(rtu_node->sensor_code, "%s-%s", buffer, str);
            // }
            // else
            // {
            //     strcpy(rtu_node->sensor_code, str);
            // }
            strcpy(rtu_node->sensor_code, str);
            CFG_LOG("sensor_code: %s \n", rtu_node->sensor_code);
        }
        else
        {
            CFG_ERROR("Not found sensor_name in %s\n", MODBUS_RTU_CFG);
            ret = -1;
            goto end;
        }

        opt = uci_lookup_option(ctx, sec, "func_mes");
        if ((NULL != opt) && (UCI_TYPE_LIST == opt->type))
        {
            struct uci_element *ListElement = NULL;
            i = 0;
            j = 0;
            uci_foreach_element(&opt->v.list, ListElement) // 遍历list
            {
                if (ListElement->name != NULL)
                {
                    i++;
                }
            }
            rtu_node->n_deal = i;
            CFG_LOG("rtu_node->n_deal %d\n", rtu_node->n_deal);
            rtu_node->funtion_code = (void **)calloc(rtu_node->n_deal, sizeof(void *));
            for (i = 0; i < rtu_node->n_deal; i++)
            {
                rtu_node->funtion_code[i] = (function_code_t *)calloc(1, sizeof(function_code_t));
            }

            uci_foreach_element(&opt->v.list, ListElement)
            {
                char *str = ListElement->name;
                function_code_t *funtion = (function_code_t *)rtu_node->funtion_code[j];
                funtion->function_id = atoi(strsep(&str, LIST_SEPARATOR));
                if (funtion->function_id < 0)
                {
                    CFG_ERROR("strsep error \n");
                    goto end;
                }
                CFG_LOG("function_id     %d\n", funtion->function_id);

                funtion->reg_addr = atoi(strsep(&str, LIST_SEPARATOR));
                if (funtion->reg_addr < 0)
                {
                    CFG_ERROR("strsep error \n");
                    goto end;
                }
                CFG_LOG("reg_addr     %d\n", funtion->reg_addr);

                funtion->reg_num = atoi(strsep(&str, LIST_SEPARATOR));
                if (funtion->reg_num < 0)
                {
                    CFG_ERROR("strsep error \n");
                    goto end;
                }
                CFG_LOG("reg_num      %d\n", funtion->reg_num);

                strcpy(funtion->name, str);
                if (funtion->name == NULL)
                {
                    CFG_ERROR("strsep error \n");
                    goto end;
                }
                CFG_LOG("name        %s\n", funtion->name);
                CFG_LOG("-------------------------\n");
                j++;
            }
        }

        list_add(&rtu_node->list, &g_modbus_rtu_list);
    }

end:
    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    if (list_empty(&g_modbus_rtu_list))
    {
        CFG_ERROR("No sensor is available \n");
        return  -1;
    }

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

    CFG_LOG("***************************\n");

    /* 判断配置文件是否存在 */
    if (access(MODBUS_SERIAL_CFG, F_OK) != 0)
    {
        CFG_LOG("Not find config %s \n", MODBUS_SERIAL_CFG);
        return -1;
    }

    /* 申请一个UCI上下文. */
    ctx = uci_alloc_context();
    if (uci_load(ctx, MODBUS_SERIAL_CFG, &pkg) != 0)
    {
        CFG_LOG("uci_load():user_list failed\n");
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
            CFG_LOG("enable:%s \n", str);
        }
        else
        {
            CFG_LOG("serial disable ! \n");
            return -1;
        }

        serial_t = (modbus_serial_t *)calloc(1, sizeof(modbus_serial_t));
        if (serial_t == NULL)
        {
            CFG_LOG("serial_t calloc failed !!!!!");
            continue;
        }
        memset(serial_t, 0, sizeof(modbus_serial_t));

        str = uci_lookup_option_string(ctx, sec, "busname");
        if (str != NULL)
        {
            strcpy(serial_t->name, str);
            CFG_LOG("serial_t->name:%s \n", str);
        }

        str = uci_lookup_option_string(ctx, sec, "serial_type");
        if (str != NULL)
        {
            CFG_LOG("serial_type:%s \n", str);
            serial_t->serial_type = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "speed");
        if (str != NULL)
        {
            CFG_LOG("speed:%s \n", str);
            serial_t->speed = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "data_bits");
        if (str != NULL)
        {
            CFG_LOG("data_bits:%s \n", str);
            serial_t->data_bits = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "stop_bits");
        if (str != NULL)
        {
            CFG_LOG("stop_bits:%s \n", str);
            serial_t->stop_bits = atoi(str);
        }

        str = uci_lookup_option_string(ctx, sec, "check_bits");
        if (str != NULL)
        {
            CFG_LOG("check_bits:%s \n", str);
            strcpy(serial_t->check_bits, str);
        }

        list_add(&serial_t->list, &g_modbus_serial_list);
    }

    uci_unload(ctx, pkg);
    uci_free_context(ctx);

    if (list_empty(&g_modbus_serial_list))
    {
        CFG_ERROR("No serial port is available \n");
        return -1;
    }

    CFG_LOG("***************************\n");

    return 0;
}

int cfg_init(void)
{
    int rc;

    g_cfg_id = dbg_module_reg("CFG");
    if (g_cfg_id < 0)
    {
        CFG_ERROR("Register cfg module failed\n");
        return -1;
    }
    CFG_LOG("debug module reg [cfg] success\n");

    rc = modbus_serial_load();
    if (rc < 0)
    {
        CFG_ERROR("modbus_serial_load  failed\n");
        return -1;
    }
    CFG_LOG("modbus_serial_load config success\n");

    rc = modbus_rtu_load();
    if (rc < 0)
    {
        CFG_ERROR("modbus_rtu_load  failed\n");
        return -1;
    }
    CFG_LOG("modbus_rtu_load config success\n");
    return 0;
}
