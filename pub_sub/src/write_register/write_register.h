#ifndef __WRITE_REGISTER_H__
#define __WRITE_REGISTER_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include <libubox/uloop.h>
#include <errno.h>
#include "modbus_init.h"

void write_register();

#endif