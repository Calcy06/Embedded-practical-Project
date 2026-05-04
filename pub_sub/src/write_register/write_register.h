#ifndef __READ_VOLUME_H__
#define __READ_VOLUME_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include <libubox/uloop.h>
#include "modbus_init/modbus_init.h"

int write_register();

#endif