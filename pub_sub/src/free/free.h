#ifndef FREE_H
#define FREE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <modbus/modbus.h> // modbus_close / modbus_free
#include <libubox/list.h>  // 链表 list_for_each_entry_safe
#include "modbus_init.h"  // 包含 arr_sensor 和 modbus_serial_t 定义
#include "read_uci.h"     // 包含 sensor_list 和 struct sensor_node 定义

extern struct list_head sensor_list;
extern modbus_serial_t arr_sensor[];

extern void free_all_resources(void);

#endif