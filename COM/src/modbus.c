// 标准C语言输入输出头文件：提供 printf、fprintf 等打印函数
#include <stdio.h>
// 标准库头文件：提供 malloc、free、atoi 等通用函数
#include <stdlib.h>
// 字符串操作头文件：提供 memcpy、memset、strlen 等内存/字符串操作函数
#include <string.h>
// Unix标准头文件：提供 close、sleep、read、write 等系统调用函数
#include <unistd.h>
// 错误码头文件：提供 errno 变量，用于获取系统函数调用失败的错误原因
#include <errno.h>

// libmodbus 库头文件：用于实现 Modbus RTU 通信（传感器/设备通信协议）
#include <modbus/modbus.h>
// libubox 事件循环头文件：提供定时器、事件驱动功能（定时读取传感器）
#include <libubox/uloop.h>

// 宏定义：日志打印宏（绿色字体输出，自带 函数名 + 行号，方便调试）
// 参数：format 格式化字符串，argument... 可变参数（和 printf 用法一样）
#define LOG(format, argument...) printf("\033[1;32m[ %s ](line %d) -> " format "\r\n\033[0m", __FUNCTION__, __LINE__, ##argument)

// 自定义结构体：Modbus 串口配置结构体（存放串口所有参数）
// 作用：统一管理 Modbus 串口的名称、波特率、数据位、校验位等配置
typedef struct modbus_serial_s
{
    char name[10];      // 串口名称（如 ttyS1、ttyUSB0），最大10个字符
    int serial_type;    // 串口类型：1=RS232 / 2=RS485
    int speed;          // 波特率（常用：9600、115200）
    int data_bits;      // 数据位（常用：8位）
    int stop_bits;      // 停止位（常用：1位）
    char check_bits[2]; // 校验位：N无校验 / E偶校验 / O奇校验
    modbus_t *ctx;      // Modbus 上下文指针（库内部使用，相当于通信句柄）
} modbus_serial_t;

// 自定义结构体：倾角传感器数据结构体
// 作用：存放传感器读取到的 X、Y、Z 三个轴的角度值
typedef struct InclinationSensor_s
{
    float angle_x; // X轴角度
    float angle_y; // Y轴角度
    float angle_z; // Z轴角度
} InclinationSensor_t;

// 全局变量：Modbus串口配置实例（存放所有串口参数）
modbus_serial_t rtuctl;
// 全局变量：倾角传感器数据实例（存放最终解析出的角度）
InclinationSensor_t incsensor;

// 全局变量：libubox 定时器结构体（用于定时读取传感器数据）
static struct uloop_timeout g_rtu_timer;

/********************************************************************
* 函数功能：数据转换函数 —— 将传感器的2字节原始数据 转为 实际角度值
* 传入参数：uint16_t input  —— 从传感器读到的原始16位数据
* 返回值：float —— 转换后的真实角度值（除以100得到小数）
********************************************************************/
float convert_to_signed_float(uint16_t input)
{
    // 把无符号整数 转为 有符号整数（支持负数角度）
    int16_t signed_value = (int16_t)input;
    // 传感器数据规则：真实值 = 原始值 / 100
    float final_value = signed_value / 100.0f;
    // 返回计算好的角度
    return final_value;
}

/********************************************************************
* 函数功能：Modbus 定时轮询函数（定时器到点自动执行）
* 作用：每隔1秒读取一次倾角传感器的寄存器数据并解析
* 传入参数：struct uloop_timeout *uloop_t —— libubox 定时器结构体
* 返回值：无
********************************************************************/
static void modbus_rtu_timed_polling(struct uloop_timeout *uloop_t)
{
    int regs = 0;           // 保存实际读到的寄存器数量
    int i = 0;              // 循环计数器（当前代码未使用）
    uint16_t buf[50] = {0}; // 数据缓冲区：存放从传感器读到的原始数据

    // 调用 libmodbus 库函数：读取传感器寄存器
    // 参数1：Modbus 上下文
    // 参数2：起始寄存器地址 0x01
    // 参数3：要读取的寄存器数量 0x03
    // 参数4：读取到的数据存入 buf 数组
    regs = modbus_read_registers(rtucl.ctx, 0x01, 0x03, buf);
    // 判断：读取失败（返回-1）
    if (regs == -1)
    {
        // 打印失败原因
        LOG("%s\n", modbus_strerror(errno));
        // 跳转到末尾，重新设置定时器
        goto end;
    }

    // 打印成功读取到的寄存器数量
    LOG("regs  %d  \n", regs);

    // 解析3个寄存器数据，转为角度值并存入结构体
    // 寄存器0 → X轴角度
    incsensor.angle_x = convert_to_signed_float(buf[0]);
    // 寄存器1 → Y轴角度
    incsensor.angle_y = convert_to_signed_float(buf[1]);
    // 寄存器2 → Z轴角度
    incsensor.angle_z = convert_to_signed_float(buf[2]);

    // 打印解析后的角度（保留2位小数）
    LOG("<%.2f°> \n", incsensor.angle_x); 
    LOG("<%.2f°> \n", incsensor.angle_y); 
    LOG("<%.2f°> \n", incsensor.angle_z); 

end:t
    // 重新绑定定时器回调函数
    uloop_t->cb = modbus_rtu_timed_polling;
    // 重新设置定时器：1000ms（1秒）后再次执行读取
    uloop_timeout_set(uloop_t, 1000);
}

/********************************************************************
* 函数功能：Modbus 串口初始化函数
* 作用：配置串口、创建Modbus连接、绑定传感器参数
* 传入参数：modbus_serial_t *node —— 传入串口配置结构体
* 返回值：int —— 成功返回0，失败返回-1
********************************************************************/
static int modbus_serial_registers(modbus_serial_t *node)
{
    int i = 0;          // 循环计数器（未使用）
    char port_name[32]; // 拼接后的完整串口路径（如 /dev/ttyS1）

    // 打印初始化开始日志
    LOG("************** modbus_serial_registers start\n");
    // 打印所有配置参数
    LOG("name:%s \n", node->name);
    LOG("serial_type:%d \n", node->serial_type);
    LOG("speed:%d \n", node->speed);
    LOG("data_bits:%d \n", node->data_bits);
    LOG("stop_bits:%d \n", node->stop_bits);
    LOG("check_bits:%s \n", node->check_bits);

    // 清空缓冲区
    memset(port_name, 0, 32);
    // 拼接完整串口设备路径：/dev/xxx
    sprintf(port_name, "/dev/%s", node->name);
    // 打印最终串口路径
    LOG("name:%s \n", port_name);

    // 创建 Modbus RTU 连接
    // 参数：串口名、波特率、校验位、数据位、停止位
    node->ctx = modbus_new_rtu(port_name, node->speed, (char)node->check_bits[0], node->data_bits, node->stop_bits);
    // 判断：创建失败
    if (node->ctx == NULL)
    {
        LOG("Connexion failed: %s\n", modbus_strerror(errno));
        return -1;
    }

    // 开启 Modbus 调试模式（打印通信数据）
    modbus_set_debug(node->ctx, TRUE);
    // 设置传感器从机地址：1
    modbus_set_slave(node->ctx, 1);

    // 设置串口硬件类型：RS232 或 RS485
    if (node->serial_type == 1)
    {
        modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS232);
    }
    else
    {
        modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS485);
    }

    // 设置 RS485 收发控制引脚
    modbus_rtu_set_rts(node->ctx, MODBUS_RTU_RTS_UP);

    // 建立连接
    if (modbus_connect(node->ctx) == -1)
    {
        LOG("Connexion failed: %s\n", modbus_strerror(errno));
        modbus_free(node->ctx); // 释放资源
        return -1;
    }

    // 设置通信超时时间：3秒
    modbus_set_response_timeout(node->ctx, 3, 0);

    // 初始化完成日志
    LOG("************** modbus_serial_registers end\n");

    // 初始化成功
    return 0;
}

/********************************************************************
* 函数功能：主函数（程序入口）
* 作用：配置参数、初始化、启动定时器、运行事件循环
* 传入参数：无（代码内部写死参数）
* 返回值：0正常退出
********************************************************************/
int main(int argc, char *argv[])
{
    // 给结构体赋值：串口名称 = ttyS1
    memcpy(rtuctl.name, "ttyS1", sizeof(rtuctl.name)-1);
    // 赋值：校验位 = N（无校验）
    memcpy(rtuctl.check_bits, "N", sizeof(rtuctl.check_bits)-1);

    // 串口类型 = 2（RS485）
    rtuctl.serial_type = 2;
    // 波特率 = 9600
    rtuctl.speed = 9600;
    // 数据位 = 8
    rtuctl.data_bits = 8;
    // 停止位 = 1
    rtuctl.stop_bits = 1;

    // 初始化 libubox 事件库
    uloop_init();
    // 清空定时器结构体
    memset(&g_rtu_timer, 0, sizeof(g_rtu_timer));

    // 调用函数：初始化 Modbus 串口
    if(modbus_serial_registers(&rtuctl) < 0){
        LOG("modbus init failed");
        return -1;
    }

    // 启动第一次定时读取
    modbus_rtu_timed_polling(&g_rtu_timer);

    // 启动事件循环（死循环，一直执行定时任务）
    uloop_run();
    // 退出循环后释放资源
    uloop_done();

    // 关闭 Modbus 连接
    modbus_close(rtuctl.ctx);
    // 释放 Modbus 上下文
    modbus_free(rtuctl.ctx);

    return 0;
}