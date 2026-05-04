#ifndef __READ_CO_H__
#define __READ_CO_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include <libubox/uloop.h>
#include "modbus_init/modbus_init.h"

int read_co();
void free_sensor_list();
void free_serial_cfg();

#endif