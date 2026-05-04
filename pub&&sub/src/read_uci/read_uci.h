#ifndef __READ_UCI_H__
#define __READ_UCI_H__

#include <uci.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libubox/uloop.h>

#define PATH_SENSOR "/root/test/pub_sub/uci_cfg/sensor"
#define PATH_SERIAL "/root/test/pub_sub/uci_cfg/serial"

extern struct list_head sensor_list;

struct sensor_node
{
    int slave;
    char *name;
    struct list_head list;
};

extern char *name; // 串口名字
extern int speed;
extern int data_bits;
extern int stop_bits;
extern char check_bits;
extern struct list_head sensor_list;

static void read_uci_sensor();
static void read_uci_serial();
void read_uci();

#endif