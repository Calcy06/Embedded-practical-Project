// ====================== 头文件 ======================
#include <stdio.h>       // 【系统内置】标准输入输出
#include <stdlib.h>      // 【系统内置】内存分配/退出
#include <string.h>      // 【系统内置】字符串/内存操作
#include <unistd.h>      // 【系统内置】延时/系统调用
#include <uci.h>         // 【第三方库】OpenWrt配置库
#include <libubus.h>     // 【第三方库】UBus通信
#include <libubox/uloop.h>//【第三方库】定时器/事件循环
#include <libubox/list.h>//【第三方库】双向链表
#include <libubox/blobmsg_json.h>//【第三方库】UBus-JSON转换
#include <memory.h>      // 【系统内置】内存操作
#include <json-c/json.h> // 【第三方库】JSON封装
#include "modbus_rtu.h"   // 【自定义】核心结构体
#include "cfg/cfg.h"     // 【自定义】配置加载
#include "ubus/ubus.h"   // 【自定义】UBus通信

// 【自定义全局变量】UBus消息缓冲区
static struct blob_buf user_list_buf;
// 【自定义外部变量】UBus对象/上下文（来自ubus.c）
extern struct ubus_object modbus_rtu_object;
extern struct ubus_context *ubus_ctx;
// 【第三方库变量】uloop定时器（定时采集）
static struct uloop_timeout g_rtu_timer;

// ====================== GGTCQ传感器数据处理 ======================
// 【核心函数】json_object_new_object/strcpy/json_object_put
// 【自定义函数】GGTCQ数据转JSON
// 参数：pos=传感器节点；msg=输出JSON字符串
static void GGTCQ_data_Deal(modbus_rtu_node_t *pos, unsigned char *msg)
{
    // 【第三方库】创建空JSON对象
	json_object *jobj = json_object_new_object();
    // 【第三方库】添加键值对：sensor=GGTCQ
	json_object_object_add(jobj, "sensor", json_object_new_string(pos->sensor_code));
    // 【第三方库】添加键值对：ill=采集值
	json_object_object_add(jobj, "ill", json_object_new_int(pos->buf[pos->reg_num - 1]));
    // 【系统内置】打印JSON
	printf("Created JSON: %s\n", json_object_to_json_string(jobj));
    // 【系统内置】拷贝JSON到输出参数
	strcpy(msg, json_object_to_json_string(jobj));
    // 【第三方库】释放JSON内存
	json_object_put(jobj);
}

// ====================== 声光报警器数据处理 ======================
// 【自定义函数】声光报警器数据转JSON
static void SGBJQ_data_Deal(modbus_rtu_node_t *pos, unsigned char *msg)
{
	json_object *jobj = json_object_new_object();
	json_object_object_add(jobj, "sensor", json_object_new_string(pos->sensor_code));
	json_object_object_add(jobj, "vol", json_object_new_int(pos->buf[pos->reg_num - 1]));
	strcpy(msg, json_object_to_json_string(jobj));
	json_object_put(jobj);
}

// ====================== 单个传感器采集 ======================
// 【核心函数】modbus_read_registers/blobmsg_add_json_from_string
// 【自定义函数】读取单个传感器Modbus数据
// 参数：pos=传感器节点（所有采集参数存在这）
static int get_sensor_data(modbus_rtu_node_t *pos)
{
	int regs = 0;                  // 实际读取寄存器数
	unsigned char msg[256] = {0};   // JSON缓存

    // 【系统内置】清空数据缓冲区
	memset(pos->buf, 0, sizeof(pos->buf));

    // 按功能码执行不同读取
	switch (pos->function_id)
	{
	case 1:
        // 【第三方库】读线圈状态
		regs = modbus_read_bits(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
		regs = regs / 2; // 数据格式适配
		break;
	case 2:
        // 【第三方库】读输入状态
		regs = modbus_read_input_bits(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
		break;
	case 3:
        // 【第三方库】读保持寄存器（最常用）
		regs = modbus_read_registers(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
		break;
	case 4:
        // 【第三方库】读输入寄存器
		regs = modbus_read_input_registers(pos->ctx, pos->reg_addr, pos->reg_num, pos->buf);
	default:
		break;
	}

    // 读取失败打印错误
	if (regs <= 0)
	{
		printf("%s\n", modbus_strerror(errno));
		return -1;
	}

    // 按传感器类型处理数据
	if (strcmp(pos->sensor_code, "GGTCQ") == 0)
	{
		GGTCQ_data_Deal(pos, msg);
	}
	else if (strcmp(pos->sensor_code, "SGBJQ") == 0)
	{
		SGBJQ_data_Deal(pos, msg);
	}

    // 【第三方库】JSON转UBus消息
	blobmsg_add_json_from_string(&user_list_buf, msg);
	return 0;
}

// ====================== 定时轮询所有传感器 ======================
// 【核心函数】list_for_each_entry_safe/ubus_notify/uloop_timeout_set
// 【自定义函数】定时器回调：轮询采集+上报
static void modbus_rtu_timed_polling(struct uloop_timeout *uloop_t)
{
	int regs = 0;
	modbus_rtu_node_t *pos, *npos;

    // 【第三方库】安全遍历传感器链表
	list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
	{
        // 未到采集时间跳过
		if (pos->timeout > 1)
		{
			pos->timeout--;
			continue;
		}
		pos->timeout = pos->period; // 重置倒计时

        // 初始化UBus消息缓冲区
		memset(&user_list_buf, 0, sizeof(struct blob_buf));
		blob_buf_init(&user_list_buf, 0);

        // 【第三方库】设置从机地址
		modbus_set_slave(pos->ctx, pos->slave);

        // 采集数据
		regs = get_sensor_data(pos);
		if (regs != 0)
		{
			blob_buf_free(&user_list_buf);
			goto end;
		}

		usleep(50000); // 【系统内置】50ms延时

        // 【第三方库】UBus消息转JSON打印
		char *str = blobmsg_format_json_indent(user_list_buf.head, true, 0);
		printf("%s\n", str);
		free(str);

        // 【第三方库】UBus主动上报数据
		ubus_notify(ubus_ctx, &modbus_rtu_object, "modbus_rtu", user_list_buf.head, -1);
	end:
		blob_buf_free(&user_list_buf);
	}

    // 【第三方库】重置定时器：1秒后再次执行
	uloop_t->cb = modbus_rtu_timed_polling;
	uloop_timeout_set(uloop_t, 1000);
}

// ====================== 单个串口初始化 ======================
// 【核心函数】modbus_new_rtu/modbus_connect
// 【自定义函数】初始化RS485/232串口
static int modbus_serial_registers(modbus_serial_t *node)
{
	char port_name[32];
    // 拼接串口设备路径（/dev/ttyXRUSB3）
	sprintf(port_name, "/dev/%s", node->name);

    // 【第三方库】创建Modbus RTU上下文
	node->ctx = modbus_new_rtu(&port_name, node->speed, (char)node->check_bits[0], node->data_bits, node->stop_bits);
    // 设置串口模式（RS232/485）
	if (node->serial_type == 1)
		modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS232);
	else
		modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS485);

    // 【第三方库】连接串口
	if (modbus_connect(node->ctx) == -1)
	{
		modbus_free(node->ctx);
		return -1;
	}
	modbus_set_response_timeout(node->ctx, 3, 0); // 设置超时
	return 0;
}

// ====================== 总初始化 ======================
// 【自定义函数】加载配置+初始化UBus
static void modbus_rtu_init(void)
{
	int rc;
	rc = cfg_init(); // 【自定义】加载UCI配置
	rc = modbus_ubus_init(); // 【自定义】初始化UBus
}

// ====================== 串口链表处理 ======================
// 【自定义函数】初始化所有串口+绑定传感器
static void modbus_serial_list_handle(void)
{
	modbus_serial_t *pos, *npos;
	modbus_rtu_node_t *rpos, *rnpos;

    // 遍历所有串口
	list_for_each_entry_safe(pos, npos, &g_modbus_serial_list, list)
	{
		modbus_serial_registers(pos); // 初始化串口
        // 给对应传感器绑定串口句柄
		list_for_each_entry_safe(rpos, rnpos, &g_modbus_rtu_list, list)
		{
			if (pos->ctx && strcmp(pos->name, rpos->serial) == 0)
				rpos->ctx = pos->ctx;
		}
	}
}

// ====================== 主函数 ======================
// 【系统内置】程序入口
int main(int argc, char *argv[])
{
	uloop_init(); // 【第三方库】初始化事件循环

    // 【第三方库】初始化全局链表
	INIT_LIST_HEAD(&g_modbus_serial_list);
	INIT_LIST_HEAD(&g_modbus_rtu_list);

	modbus_rtu_init();          // 初始化配置+UBus
	modbus_serial_list_handle();// 初始化串口+绑定传感器

	modbus_rtu_timed_polling(&g_rtu_timer); // 启动定时采集
	uloop_run();  // 启动事件循环（程序永久运行）
	uloop_done(); // 退出清理

	modbus_ubus_done(); // 【自定义】关闭UBus
	return 0;
}