#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <uci.h>
#include <libubus.h>
#include <libubox/uloop.h>
#include <libubox/list.h>
#include <libubox/blobmsg_json.h>
#include <memory.h>
#include <json-c/json.h> 

#include "modbus_rtu.h"
#include "cfg/cfg.h"
#include "ubus/ubus.h"

static struct blob_buf user_list_buf;
extern struct ubus_object modbus_rtu_object;
extern struct ubus_context *ubus_ctx;

static struct uloop_timeout g_rtu_timer;

//  GGCTQ 数据处理
static void GGTCQ_data_Deal(modbus_rtu_node_t *pos, unsigned char *msg)
{

    json_object *jobj = json_object_new_object();  // 创建一个新的 JSON 对象
    json_object_object_add(jobj, "sensor", json_object_new_string(pos->sensor_code));
    json_object_object_add(jobj, "ill", json_object_new_int(pos->buf[pos->reg_num - 1]));
    printf("Created JSON: %s\n", json_object_to_json_string(jobj));// 这里可以将 jobj 转换为字符串并发送到终端进行提示
    strcpy(msg, json_object_to_json_string(jobj));
    json_object_put(jobj);
}

//  SGBJQ 数据处理
static void SGBJQ_data_Deal(modbus_rtu_node_t *pos, unsigned char *msg)
{
    json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "sensor", json_object_new_string(pos->sensor_code));
    json_object_object_add(jobj, "vol", json_object_new_int(pos->buf[pos->reg_num - 1]));
    strcpy(msg, json_object_to_json_string(jobj));
    json_object_put(jobj);
}

// 获取传感器数据
static int get_sensor_data(modbus_rtu_node_t *pos)
{

    int regs = 0;
    unsigned char msg[256] = {0};

    memset(pos->buf, 0, sizeof(pos->buf));

    // 这边具体对功能码做不同的下发区分
    switch (pos->function_id)
    {
    case 1:
        // 读取线圈状态取得一组逻辑线圈的当前状态（ON/OFF)   ppos->buf 应该为（u8 但实际传入u16 解析注意）
        // eg: 1 0 0 0 0 0 0 1 会变成 0x0100 0x0000 0x0000 0x0001  传入lua解析函数中，解析
        regs = modbus_read_bits(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
        regs = regs / 2; // 存入的是 u16数组 ，特殊处理 不然打印有问题
        break;
    case 2:
        // 读取输入状态 取得一组开关输入的当前状态
        regs = modbus_read_input_bits(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
        break;
    case 3:
        // 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
        regs = modbus_read_registers(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
        break;

    case 4:
        // 读取输入寄存器 在一个或多个输入寄存器中取得当前的二进制值
        regs = modbus_read_input_registers(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);

    default:
        break;
    }

    if (regs <= 0)
    {
        printf("%s\n", modbus_strerror(errno));
        /*设备无数据*/
        return -1;
    }

    if (strcmp(pos->sensor_code, "GGTCQ") == 0)
    {
        GGTCQ_data_Deal(pos, msg);
    }
    else if (strcmp(pos->sensor_code, "SGBJQ") == 0)
    {
        SGBJQ_data_Deal(pos, msg);
    }

    blobmsg_add_json_from_string(&user_list_buf, msg);

    return 0;
}

// 定时轮询所有传感器数据
static void modbus_rtu_timed_polling(struct uloop_timeout *uloop_t)
{
    int regs = 0;
    modbus_rtu_node_t *pos, *npos;
    list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
    {
        if (pos->timeout > 1)
        {
            pos->timeout--;
            continue;
        }
        pos->timeout = pos->period;

        printf("-------------------------- \n");
        printf("sensor_id:%s \n", pos->sensor_code);
        printf("serial:%s \n", pos->serial);
        printf("pos->period:%d\n", pos->period);
        memset(&user_list_buf, 0, sizeof(struct blob_buf));
        blob_buf_init(&user_list_buf, 0);
        modbus_set_slave(pos->ctx, pos->slave);

        regs = get_sensor_data(pos);
        if (regs != 0)
        {
            blob_buf_free(&user_list_buf);
            goto end;
        }
        usleep(50000); // 给个5ms的延时，防止访问的太快，来不及反应

        char *str = blobmsg_format_json_indent(user_list_buf.head, true, 0);
        printf("%s\n", str);
        free(str);
        str = NULL;

        // 调用ubus接口，将数据发送到服务器
        ubus_notify(ubus_ctx, &modbus_rtu_object, "modbus_rtu", user_list_buf.head, -1);
    end:
        // 设备没读到数据或者异常直接就准备下一个
        blob_buf_free(&user_list_buf);
    }

    // 重置定时器
    uloop_t->cb = modbus_rtu_timed_polling;
    uloop_timeout_set(uloop_t, 1000);
}

// 单个串口初始化
static int modbus_serial_registers(modbus_serial_t *node)
{
    int i = 0;
    char port_name[32];

    printf("************** modbus_serial_registers start\n");
    printf("name:%s \n", node->name);
    printf("enable:%s \n", node->enable);
    printf("serial_type:%d \n", node->serial_type);
    printf("speed:%d \n", node->speed);
    printf("data_bits:%d \n", node->data_bits);
    printf("stop_bits:%d \n", node->stop_bits);
    printf("check_bits:%s \n", node->check_bits);
    printf("port:%d \n", node->port);

    memset(&port_name, 0, 32);
    // 拼接串口设备路径（/dev/ttyXRUSB3）
    if (node->name != NULL || atoi(node->enable) != 0)
    {
        sprintf(port_name, "/dev/%s", node->name);
        printf("name:%s \n", port_name);
    }
    else
    {
        printf("name:%s  disenale open\n", port_name);
        return -1;
    }

    // 打开端口: 端口，波特率，校验位'N'，数据位，停止位 node->check_bits : 'N' 'E' 'O'
    node->ctx = modbus_new_rtu(&port_name, node->speed, (char)node->check_bits[0], node->data_bits, node->stop_bits);
    if (node->ctx == NULL)
    {
        printf("Connexion failed: %s\n", modbus_strerror(errno));
        return -1;
    }
    // 设置debug模式
    modbus_set_debug(node->ctx, false);
    // 启机默认设置为1
    modbus_set_slave(node->ctx, 1);
    // 设置串口模式(可选) 1:232 2:485 3:tcp
    if (node->serial_type == 1)
    {
        modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS232);
    }
    else
    {
        modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS485);
    }
    // 设置RTS(可选)
    modbus_rtu_set_rts(node->ctx, MODBUS_RTU_RTS_UP);

    // 建立连接
    if (modbus_connect(node->ctx) == -1)
    {
        printf("Connexion failed: %s\n", modbus_strerror(errno));
        modbus_free(node->ctx);
        return -1;
    }
    // 设置应答延时(可选)
    modbus_set_response_timeout(node->ctx, 3, 0);

    printf("************** modbus_serial_registers end\n");
}

// 总初始化
static void modbus_rtu_init(void)
{
    int rc;

    rc = cfg_init();
    if (rc != 0)
    {
        printf("cfg load	failed, exit now!\n");
        exit(1);
    }

    rc = modbus_ubus_init();
    if (rc != 0)
    {
        printf("modbus_rtu_init  failed, exit now!\n");
        exit(1);
    }
}

// 串口链表处理:初始化所有串口+绑定传感器
static void modbus_serial_list_handle(void)
{
    int rc = 0;
    if (list_empty(&g_modbus_serial_list))
    {
        return;
    }

    modbus_serial_t *pos, *npos;
    modbus_rtu_node_t *rpos, *rnpos;

    list_for_each_entry_safe(pos, npos, &g_modbus_serial_list, list)
    {
        if (modbus_serial_registers(pos) < 0)
        {
            printf("%s register fail\n", pos->name);
        }

        /* 相应的传感器节点赋值串口文件标识符 */
        list_for_each_entry_safe(rpos, rnpos, &g_modbus_rtu_list, list)
        {
            if (pos->ctx && strcmp(pos->name, rpos->serial) == 0)
            {
                rpos->ctx = pos->ctx; // 给对应传感器绑定串口句柄
            }
        }
    }
}

int main(int argc, char *argv[])
{
    uloop_init(); // 初始化事件循环

    INIT_LIST_HEAD(&g_modbus_serial_list);
    INIT_LIST_HEAD(&g_modbus_rtu_list);

    (void)modbus_rtu_init(); // 初始化配置+UBus

    (void)modbus_serial_list_handle();  // 初始化串口+绑定传感器

    /* 启动定时器 */
    modbus_rtu_timed_polling(&g_rtu_timer);
    uloop_run();
    uloop_done();
    modbus_ubus_done();

    return 0;
}
