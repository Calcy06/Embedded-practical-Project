#include <stdio.h>    // 标准输入输出头文件，用于printf、perror等打印函数
#include <stdlib.h>   // 标准库头文件，提供内存分配、程序退出等功能
#include <string.h>   // 字符串操作头文件，用于memset等内存初始化函数
#include <unistd.h>   // Unix标准头文件，提供close、sleep等系统调用
#include <errno.h>    // 错误码头文件，用于获取系统调用的错误信息
#include <modbus/modbus.h>  // Modbus协议库头文件，提供Modbus RTU通信的所有API
#include <libubox/uloop.h>  // OpenWrt/嵌入式事件循环库头文件，提供定时器、事件管理

// ====================== 宏定义 ======================
// 串口设备节点：Linux系统下USB转串口的设备路径，用于指定Modbus通信的物理端口
#define SERIAL_PORT "/dev/ttyUSB0"
// 串口波特率：Modbus RTU通信的速率，必须和从机设备配置一致
#define BAUD_RATE 9600
// Modbus从机地址：指定要通信的外部设备地址，范围1~247
#define SLAVE_ADDR 1
// 报警灯光寄存器地址：Modbus寄存器地址，控制设备灯光开关的寄存器位置
#define ALARM_REG_ADDR 0x0001 // 灯光寄存器

// Modbus上下文句柄：全局变量，存储Modbus连接的所有配置和状态
modbus_t *ctx;

// uloop 定时器结构体：全局定时器对象，用于实现周期性灯光闪烁
static struct uloop_timeout g_alarm_timer;

// ====================== 函数定义 ======================
/**
 * @brief  初始化Modbus RTU连接
 * @return 成功返回0，失败返回-1
 * @note   负责创建Modbus上下文、配置串口、设置从机地址、建立连接
 */
int modbus_alarm_init(void)
{
    // 创建Modbus RTU上下文
    // 参数：串口路径、波特率、校验位(N无校验)、数据位8、停止位1
    ctx = modbus_new_rtu(SERIAL_PORT, BAUD_RATE, 'N', 8, 1);
    if (!ctx) {  // 上下文创建失败判断
        perror("modbus_new_rtu failed");
        return -1;
    }

    // 设置Modbus从机地址
    modbus_set_slave(ctx, SLAVE_ADDR);
    // 设置Modbus响应超时时间：5秒0微秒，防止通信阻塞
    modbus_set_response_timeout(ctx, 5, 0);
    // 建立Modbus物理连接（打开串口）
    if (modbus_connect(ctx) == -1) {
        perror("modbus_connect failed");
        modbus_free(ctx);  // 释放上下文资源
        return -1;
    }
    printf("Modbus 连接成功，设备地址：%d\n", SLAVE_ADDR);
    return 0;
}

// ====================== 只改这个函数 ======================
/**
 * @brief  控制报警器灯光（纯灯光，无蜂鸣器）
 * @param  enable：控制参数 1=亮灯，0=熄灯
 * @return 无返回值
 * @note   通过Modbus写寄存器指令控制设备灯光
 */
void alarm_control(int enable)
{
    int ret;  // 存储Modbus指令执行返回值

    if (enable) {
        // 亮灯：向指定寄存器写入值0x0001
        ret = modbus_write_register(ctx, ALARM_REG_ADDR, 0x0001);
    } else {
        // 熄灯：向指定寄存器写入值0x0000
        ret = modbus_write_register(ctx, ALARM_REG_ADDR, 0x0000);
    }

    if (ret == -1) {  // Modbus写操作失败判断
        perror("亮灯控制失败");
        return;
    }
    printf("灯光状态：%s\n", enable ? "✅ 亮灯" : "❌ 熄灯");
}
// ============================================================

/**
 * @brief  定时器回调函数：定时器到期后自动执行
 * @param  t：触发回调的定时器对象指针
 * @return 无返回值
 * @note   实现灯光闪烁逻辑：每次触发翻转灯光状态，并重设定时器
 */
static void alarm_timer_cb(struct uloop_timeout *t)
{
    static int state = 0;  // 静态变量，保存灯光当前状态（0=灭，1=亮）
    state = !state;        // 状态翻转：0变1，1变0

    alarm_control(state);  // 根据翻转后的状态控制灯光

    uloop_timeout_set(t, 1000);  // 重设定时器，1000ms=1秒后再次触发
}

/**
 * @brief  主函数：程序入口
 * @return 程序退出状态码
 * @note   初始化资源、启动事件循环、程序退出时释放资源
 */
int main(void)
{
    // 初始化Modbus连接，失败则直接退出程序
    if (modbus_alarm_init() < 0) {
        return -1;
    }

    uloop_init();  // 初始化uloop事件循环库
    // 定时器结构体清零，防止脏数据
    memset(&g_alarm_timer, 0, sizeof(g_alarm_timer));
    // 绑定定时器回调函数：定时器到期执行alarm_timer_cb
    g_alarm_timer.cb = alarm_timer_cb;

    // 启动定时器：1秒后第一次触发
    uloop_timeout_set(&g_alarm_timer, 1000);

    printf("uloop 灯光闪烁程序已启动，无限循环中...\n");

    uloop_run();  // 启动uloop事件循环，程序在此处无限循环运行

    // ====================== 程序退出时执行 ======================
    uloop_done();        // 关闭uloop事件循环
    alarm_control(0);    // 关闭灯光，保证程序退出后灯光熄灭
    modbus_close(ctx);   // 关闭Modbus连接，释放串口资源
    modbus_free(ctx);    // 释放Modbus上下文内存

    return 0;
}