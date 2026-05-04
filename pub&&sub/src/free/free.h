#ifndef __FREE_H__
#define __FREE_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <modbus/modbus.h> // modbus_close / modbus_free
#include <libubox/list.h>  // 链表 list_for_each_entry_safe

void free_all_resources();

#endif