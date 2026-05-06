#ifndef __READ_UCI_H__
#define __READ_UCI_H__

#include <uci.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <libubox/uloop.h>

#define CFG_SENSOR "/root/zyh/pub_sub/uci_cfg/sensor"
#define CFG_SERIAL "/root/zyh/pub_sub/uci_cfg/serial"

struct list_head sensor_list;

struct my_node {
    int slave;
    char *name;
    struct list_head list;
};

extern char *name;
extern int speed;
extern int data_bits;
extern int stop_bits;
extern char check_bits;
extern struct list_head sensor_list;

static void read_uci_sensor();
static void read_uci_serial();
void read_uci();

#endif