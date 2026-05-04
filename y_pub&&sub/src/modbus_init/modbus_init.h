#ifndef __MODBUS_INIT_H__
#define __MODBUS_INIT_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus/modbus.h>
#include <libubox/uloop.h>
#include "read_uci/read_uci.h"

typedef struct modbus_serial_s
{
    char name[64];      // 名称，最大长度为10个字符
    int speed;          // 波特率，例如9600, 115200等
    int data_bits;      // 数据位数，例如8, 7等
    int stop_bits;      // 停止位数，例如1, 2等
    char check_bits; // 校验位，通常为'N'（无校验），'E'（偶校验），'O'（奇校验）
    int slave;
    char sensor_name[64];
    modbus_t *ctx;      // 指向Modbus上下文的指针，用于Modbus通信
} modbus_serial_t;

#define SIZE 10

extern modbus_serial_t rtuctl[SIZE];

int modbus_serial_registers();

#endif


//name: 串口名称
//sensor_name: 传感器名称