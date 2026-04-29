#ifndef __MODBUS_ALARM_H__
#define __MODBUS_ALARM_H__

#include <stdint.h>

// 报警器状态结构体
typedef struct {
    uint16_t volume;    // 音量
    uint16_t mode;      // 循环模式
    uint16_t status;    // 0停止/1播放/2暂停
    uint16_t track;     // 当前曲目
} alarm_status_t;

int modbus_alarm_init(void);
void modbus_alarm_read(alarm_status_t *st);
void modbus_alarm_play(int on);

#endif