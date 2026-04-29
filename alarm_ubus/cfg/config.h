#ifndef __CFG_CONFIG_H__
#define __CFG_CONFIG_H__

// Modbus 配置
#define MODBUS_SERIAL_DEV   "/dev/ttyUSB0"
#define MODBUS_BAUD         9600
#define MODBUS_SLAVE_ADDR   1

// 轮询间隔(ms)
#define MODBUS_POLL_INTERVAL 3000

#endif