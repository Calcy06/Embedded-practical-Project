#ifndef __MODBUS_RTU_H__  // 【内置宏】防止头文件重复包含
#define __MODBUS_RTU_H__

#include <libubus.h>      // 【第三方库】UBus头文件
#include <libubox/list.h> // 【第三方库】双向链表头文件
#include <modbus/modbus.h>// 【第三方库】Modbus库头文件

// 【自定义宏】配置文件路径（串口配置、传感器配置）
#define MODBUS_SERIAL_CFG "/root/test/modbus_rtu_test/etc/config/modbus_serial"
#define MODBUS_RTU_CFG "/root/test/modbus_rtu_test/etc/config/modbus_rtu"

// 【自定义结构体】串口配置结构体（存储串口所有参数）
typedef struct modbus_serial_s
{
    struct list_head list;      // 【第三方库】链表节点（用于串成链表）
    char name[32];              // 串口名（ttyXRUSB3）
    char enable[2];             // 串口使能（1启用/0禁用）
    int serial_type;            // 串口类型（1=RS232/2=RS485）
    int speed;                  // 波特率（9600/38400）
    int data_bits;              // 数据位（8）
    int stop_bits;              // 停止位（1）
    char check_bits[2];         // 校验位（N无/E偶/O奇）
    int port;                   // 端口号
    modbus_t *ctx;              // 【第三方库】Modbus上下文（串口句柄）
} modbus_serial_t;

// 【自定义结构体】传感器节点结构体（存储单个传感器所有参数）
typedef struct modbus_rtu_node_s
{
    struct list_head list;      // 【第三方库】链表节点
    char serial[15];            // 绑定的串口名（和上面串口对应）
    bool r_enable;              // 读使能（是否采集）
    char sensor_code[60];       // 传感器编码（CH4/SGBJQ）
    int slave;                  // Modbus从机地址（1-247）
    int period;                 // 采集周期（秒）
    int timeout;                // 倒计时（到0就采集）
    int reg_num;                // 要读取的寄存器数量
    int reg_addr;               // 寄存器起始地址
    int function_id;            // Modbus功能码（1/2/3/4）
    uint16_t buf[32];           // 存储采集的原始数据
    modbus_t *ctx;              // 继承的串口句柄
} modbus_rtu_node_t;

// 【自定义全局变量】双向链表（存储所有传感器、所有串口）
struct list_head g_modbus_rtu_list;
struct list_head g_modbus_serial_list;

#endif /* __MODBUS_RTU_H__ */