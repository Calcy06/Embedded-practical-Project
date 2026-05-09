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

#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

#include "modbus_rtu_debug.h"
#include "modbus_rtu.h"
#include "cfg/cfg.h"
#include "ubus/ubus.h"
#include "tool/data_deal.h"

#define DATA_DEAL_FILE "/usr/local/lib/lua/modbus_rtu/modbus_data_deal"

static struct blob_buf user_list_buf;
lua_State *L;
extern struct ubus_object modbus_rtu_object;
extern struct ubus_context *ubus_ctx;

static struct uloop_timeout g_rtu_timer;
dbg_md_t g_main_id = -1;
static struct ubus_context *g_ubus_ctx;

static int modbus_rtu_set_slave(modbus_rtu_node_t *node);

enum
{
	MODE,
	ATTR,
	MAX,
};

static const struct blobmsg_policy set_policy[MAX] = {
	[MODE] = {.name = "mode", .type = BLOBMSG_TYPE_STRING},
	[ATTR] = {.name = "attr", .type = BLOBMSG_TYPE_STRING},

};

int modbus_set_period(struct ubus_context *ctx, struct ubus_object *obj,
					  struct ubus_request_data *req, const char *method, struct blob_attr *msg)
{
	struct blob_attr *tb[MAX];
	modbus_rtu_node_t *pos, *npos;

	blobmsg_parse(set_policy, MAX, tb, blob_data(msg), blob_len(msg));

	if (tb[MODE] != NULL)
	{
		if (tb[ATTR] == NULL)
		{
			/* 错误处理 */
			return 0;
		}
		int mode = atoi(blobmsg_get_string(tb[MODE]));
		char *sensor = blobmsg_get_string(tb[ATTR]);
		/*查询设备ID */
		list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
		{
			if (strcmp(pos->sensor_code, sensor) == 0)
			{
				if (mode == 1)
				{
					RTU_LOG("%s set Iot mode.\n", sensor);
					pos->period = pos->tempperiod;
					pos->timeout = pos->period;
				}
				else if ((mode == 0))
				{
					RTU_LOG("%s set chart mode.\n", sensor);
					pos->period = 1; // 设置为1s;
					pos->timeout = 0;
				}
				break;
			}
		}
	}

	return 0;
}

static int get_sensor_data(modbus_rtu_node_t *pos, function_code_t *ppos)
{

	void *data;
	int regs = 0;
	int i;

	memset(ppos->buf, 0, sizeof(ppos->buf));

	// 这边具体对功能码做不同的下发区分
	switch (ppos->function_id)
	{
	case 1:
		// 读取线圈状态取得一组逻辑线圈的当前状态（ON/OFF)   ppos->buf 应该为（u8 但实际传入u16 解析注意）
		// eg: 1 0 0 0 0 0 0 1 会变成 0x0100 0x0000 0x0000 0x0001  传入lua解析函数中，解析
		regs = modbus_read_bits(pos->ctx, ppos->reg_addr, ppos->reg_num, ppos->buf);
		regs = regs / 2; //存入的是 u16数组 ，特殊处理 不然打印有问题
		break;
	case 2:
		// 读取输入状态 取得一组开关输入的当前状态
		regs = modbus_read_input_bits(pos->ctx, ppos->reg_addr, ppos->reg_num, ppos->buf);
		break;
	case 3:
		// 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
		regs = modbus_read_registers(pos->ctx, ppos->reg_addr, ppos->reg_num, ppos->buf);
		break;

	case 4:
		// 读取输入寄存器 在一个或多个输入寄存器中取得当前的二进制值
		regs = modbus_read_input_registers(pos->ctx, ppos->reg_addr, ppos->reg_num, ppos->buf);

	default:
		break;
	}

	if (regs <= 0)
	{
		RTU_LOG("%s\n", modbus_strerror(errno));
		/*设备无数据*/
		return -1;
	}

	// printf("regs  %d  \n", regs);
	// for (i = 0; i < regs; ++i)
	// {
	// 	printf("<%#x> \n", ppos->buf[i]);
	// }

	/* 将数据处理的函数压入堆栈 */
	lua_getglobal(L, ppos->name);
	/* 创建一个表与lua交互 */
	lua_newtable(L);

	/* 将参数全部压入堆栈 eg: 1(序号) : 21  2: 22 3 :33 */
	for (i = 0; i < regs; i++)
	{
		lua_pushnumber(L, i + 1);
		lua_pushnumber(L, ppos->buf[i]);
		lua_settable(L, -3);
	}

	/* 调用函数并处理 */
	lua_pcall(L, 1, 1, 0);
	const char *dealdata = lua_tostring(L, -1);
	char datastring[1024];
	strcpy(datastring, dealdata);
	// printf("Get data %s\n", datastring);

	blobmsg_add_json_from_string(&user_list_buf, datastring);

	return 0;
}

static void modbus_rtu_timed_polling(struct uloop_timeout *uloop_t)
{
	if (list_empty(&g_modbus_rtu_list))
	{
		goto end;
	}

	int regs = 0;
	int i;

	modbus_rtu_node_t *pos, *npos;

	void *c, *b, *e;
	list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
	{
		if (pos->timeout > 1)
		{
			pos->timeout--;
			continue;
		}
		pos->timeout = pos->period;

		RTU_LOG("-------------------------- \n");
		// RTU_LOG("sensor_name:%s \n", pos->sensor_name);
		if (pos->r_enable == false)
		{
			RTU_ERROR("%s forbid read\n", pos->sensor_code);
			continue;
		}

		RTU_LOG("serial:%s \n", pos->serial);
		RTU_LOG("sensor_id:%s \n", pos->sensor_code);
		RTU_LOG("pos->period:%d\n", pos->period);

		memset(&user_list_buf, 0, sizeof(struct blob_buf));
		blob_buf_init(&user_list_buf, 0);
		b = blobmsg_open_array(&user_list_buf, pos->sensor_code);
		e = blobmsg_open_table(&user_list_buf, NULL);
		blobmsg_add_u64(&user_list_buf, "ts", Get_time_stam());
		c = blobmsg_open_table(&user_list_buf, "values");

		modbus_rtu_set_slave(pos);

		for (i = 0; i < pos->n_deal; i++)
		{
			// 根据功能码执行相应功能
			function_code_t *ppos = (function_code_t *)pos->funtion_code[i];
			RTU_LOG("ppos[%d].function_id:%d  data_name %s\n", i, ppos->function_id, ppos->name);
			switch (ppos->function_id)
			{
			case 1:
			case 2:
			case 3:
			case 4:
				// 读取保持寄存器 在一个或多个保持寄存器中取得当前的二进制值
				regs = get_sensor_data(pos, ppos);
				if (regs != 0)
				{
					blob_buf_free(&user_list_buf);
					goto end;
				}
				break;
			default:
				RTU_ERROR("func_id no found\n");
				goto end;
			}
			usleep(50000); // 给个5ms的延时，防止访问的太快，来不及反应
		}

		blobmsg_close_table(&user_list_buf, c);
		blobmsg_close_table(&user_list_buf, e);
		blobmsg_close_array(&user_list_buf, b);

		char *str = blobmsg_format_json_indent(user_list_buf.head, true, 0);
		RTU_LOG("%s\n", str);
		free(str);
		str = NULL;

		// 调用ubus接口，将数据发送到服务器
		ubus_notify(ubus_ctx, &modbus_rtu_object, "modbus_rtu", user_list_buf.head, -1);
end:
		//设备没读到数据或者异常直接就准备下一个
		blob_buf_free(&user_list_buf);
	}

	// 重置定时器
	uloop_t->cb = modbus_rtu_timed_polling;
	uloop_timeout_set(uloop_t, 1000);
}

static int modbus_rtu_set_slave(modbus_rtu_node_t *node)
{
	int i = 0;
	modbus_serial_t *pos, *npos;

	list_for_each_entry_safe(pos, npos, &g_modbus_serial_list, list)
	{
		if (strcmp(pos->name, node->serial) == 0)
		{
			modbus_set_slave(node->ctx, node->slave);
			return 0;
		}
	}
}

static int modbus_serial_registers(modbus_serial_t *node)
{
	int i = 0;
	char port_name[32];

	RTU_LOG("************** modbus_serial_registers start\n");
	RTU_LOG("name:%s \n", node->name);
	RTU_LOG("enable:%s \n", node->enable);
	RTU_LOG("serial_type:%d \n", node->serial_type);
	RTU_LOG("speed:%d \n", node->speed);
	RTU_LOG("data_bits:%d \n", node->data_bits);
	RTU_LOG("stop_bits:%d \n", node->stop_bits);
	RTU_LOG("check_bits:%s \n", node->check_bits);
	RTU_LOG("port:%d \n", node->port);

	memset(&port_name, 0, 32);
	if (node->name != NULL || atoi(node->enable) != 0)
	{
		sprintf(port_name, "/dev/%s", node->name);
		RTU_LOG("name:%s \n", port_name);
	}
	else
	{
		RTU_LOG("name:%s  disenale open\n", port_name);
		return -1;
	}

	// 打开端口: 端口，波特率，校验位'N'，数据位，停止位 node->check_bits : 'N' 'E' 'O'
	node->ctx = modbus_new_rtu(&port_name, node->speed, (char)node->check_bits[0], node->data_bits, node->stop_bits);
	if (node->ctx == NULL)
	{
		RTU_LOG("Connexion failed: %s\n", modbus_strerror(errno));
		return -1;
	}
	// 设置debug模式
	modbus_set_debug(node->ctx, TRUE);
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
		RTU_LOG("Connexion failed: %s\n", modbus_strerror(errno));
		modbus_free(node->ctx);
		return -1;
	}
	// 设置应答延时(可选)
	//  modbus_set_response_timeout(node->ctx, 0, 1000000);
	modbus_set_response_timeout(node->ctx, 3, 0);

	RTU_LOG("************** modbus_serial_registers end\n");
}

static int modbus_rtu_debug_init(void)
{
	if (dbg_init("modbus_rtu", LOG_FILE_PATH, LOG_FILE_SIZE) != 0)
	{
		fprintf(stderr, "ERROR: debug init failed in %s on %d lines\n", __FILE__, __LINE__);
		return -1;
	}
	RTU_LOG("dbg_init() success\n");

	g_main_id = dbg_module_reg("main");
	if (g_main_id < 0)
	{
		fprintf(stderr, "ERROR: register debug module failed in %s on %d lines\n", __FILE__, __LINE__);
		return -1;
	}
	RTU_LOG("dbg_module_reg(main) success\n");

	return 0;
}

static void modbus_rtu_init(void)
{
	int rc;

	rc = modbus_rtu_debug_init();
	if (rc != 0)
	{
		RTU_ERROR("modbus_rtu_debug_init  failed, exit now!\n");
		exit(1);
	}

	rc = cfg_init();
	if (rc != 0)
	{
		RTU_ERROR("cfg load	failed, exit now!\n");
		exit(1);
	}

	rc = modbus_ubus_init();
	if (rc != 0)
	{
		RTU_ERROR("modbus_rtu_init  failed, exit now!\n");
		exit(1);
	}
}

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
			RTU_ERROR("%s register fail\n", pos->name);
		}

		/* 相应的传感器节点赋值串口文件标识符 */
		list_for_each_entry_safe(rpos, rnpos, &g_modbus_rtu_list, list)
		{
			if (pos->ctx && strcmp(pos->name, rpos->serial) == 0)
			{
				rpos->ctx = pos->ctx;
			}
		}
	}
}

int main(int argc, char *argv[])
{
	L = luaL_newstate();
	luaL_openlibs(L);
	RTU_LOG("************* modbus_rtu start ****************\n");

	int result = luaL_dofile(L, DATA_DEAL_FILE);
	if (result != 0)
	{
		printf("Could not load Lua File %s, exiting\n", DATA_DEAL_FILE);
		lua_close(L);
		return -1;
	}

	uloop_init();

	INIT_LIST_HEAD(&g_modbus_serial_list);
	INIT_LIST_HEAD(&g_modbus_rtu_list);

	(void)modbus_rtu_init();

	(void)modbus_serial_list_handle();
	//(void)modbus_rtu_list_handle();

	/* 启动定时器 */
	modbus_rtu_timed_polling(&g_rtu_timer);

	uloop_run();
	uloop_done();
	modbus_ubus_done();
	RTU_LOG("************* modbus_rtu exit ****************\n");

	return 0;
}
