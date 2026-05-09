#ifndef __MODBUS_RTU_H__
#define __MODBUS_RTU_H__

#include <libubus.h>
#include <libubox/list.h>
#include <modbus/modbus.h>

#define MODBUS_SERIAL_CFG "/etc/config/modbus_serial"
#define MODBUS_RTU_CFG "/etc/config/modbus_rtu"
#define LOG_FILE_PATH "/tmp/modbus_rtu.log"
#define LOG_FILE_SIZE 50 /* 单位：KB */

#define MODBUS_SENSOR_NO_ERROR 0x00
#define MODBUS_SENSOR_CRC_ERROR 0x03
#define MODBUS_SENSOR_DATA_ERROR 0x04

#define SENSOR_ENBLE "1"
#define SENSOR_DISENBLE "0"
#define M_IEEE_754 "1"
#define M_DEFAULT "2"

typedef enum deal_method
{
    D_IEEE_754 = 1,
    D_DEFAULT,
} d_md;

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

typedef struct function_code_s
{
    unsigned int function_id; // 功能码
    unsigned int reg_num;     // 寄存器数量
    unsigned int reg_addr;    // 寄存器起始地址
    uint16_t buf[32];         // 存储采集的原始数据
    char name[32];            // 寄存器数据处理函数

} function_code_t;

typedef struct modbus_rtu_node_s
{
    struct list_head list;
    char serial[15];       // 接口
    bool r_enable;         // 读使能开关
    char sensor_code[60];  // 出厂编码
    int slave;             // 设备id，从机号
    int period;            // 周期
    int tempperiod;        // 临时存储原先的周期数据
    int timeout;           // 还有多长时间可以采集
    int n_deal;            // 传感器采集项
    u_int32_t status_code; // 传感器状态码
    char methods[15];      // 记录数据处理的方法,方便调用lua中的函数
    void **funtion_code;   // 功能码的集合，包括了读写
    modbus_t *ctx;

} modbus_rtu_node_t;

struct list_head g_modbus_rtu_list;
struct list_head g_modbus_serial_list;

int modbus_set_period(struct ubus_context *ctx, struct ubus_object *obj,
                      struct ubus_request_data *req, const char *method, struct blob_attr *msg);

#endif /* __MODBUS_RTU_H__ */
