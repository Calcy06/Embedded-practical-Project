#ifndef __UBUS_ALARM_H__
#define __UBUS_ALARM_H__

#include "modbus.h"

int ubus_alarm_init(void);
void ubus_alarm_publish(alarm_status_t *st);

#endif