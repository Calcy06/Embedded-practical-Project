#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#include <libubus.h>
#include <libubox/list.h>
#include <modbus/modbus.h>

#define MODBUS_SERIAL_CFG "/home/firefly/Desktop/modbus_rtu/etc/config/modbus_serial"
#define MODBUS_RTU_CFG "/home/firefly/Desktop/modbus_rtu/etc/config/modbus_rtu"

typedef struct modbus_serial_s
{
    struct list_head list;
    char name[32];
    char enable[2];
    int serial_type;
    int speed;
    int data_bits;
    int stop_bits;
    char check_bits[2];
    int port;
    modbus_t *ctx;
} modbus_serial_t;

typedef struct modbus_rtu_node_s
{
    struct list_head list;
    char serial[15];      // 接口
    bool r_enable;        // 读使能开关
    char sensor_code[60]; // 出厂编码
    int slave;            // 设备id，从机号
    int period;           // 周期
    int timeout;          // 还有多长时间可以采集
    int reg_num;          // 寄存器数量
    int reg_addr;         // 寄存器起始地址
    int function_id;      // 功能码
    uint16_t buf[32];     // 存储采集的原始数据
    modbus_t *ctx;

} modbus_rtu_node_t;

struct list_head g_modbus_rtu_list;
struct list_head g_modbus_serial_list;

#endif /* __MODBUS_RTU_H__ */
