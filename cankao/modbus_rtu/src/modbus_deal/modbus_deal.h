#ifndef __MODBUS_DEAL_H__
#define __MODBUS_DEAL_H__

#include <modbus/modbus.h>
#include "modbus_rtu.h"

int modbus_cmd_parse(unsigned char *buffer);

#endif /*__MODBUS_DEAL_H__*/