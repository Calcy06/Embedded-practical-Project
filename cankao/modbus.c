#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <modbus.h>
#include <modbus-rtu.h>
#include <libubox/uloop.h>
#include <errno.h>

#define LOG(format, argument...) printf("\033[1;32m[ %s ](line %d) -> " format "\r\n\033[0m", __FUNCTION__, __LINE__, ##argument)

typedef struct modbus_serial_s
{
    char name[10];      // 名称，最大长度为10个字符
    int serial_type;    // 串口类型，例如RS232, RS485等
    int speed;          // 波特率，例如9600, 115200等
    int data_bits;      // 数据位数，例如8, 7等
    int stop_bits;      // 停止位数，例如1, 2等
    char check_bits[2]; // 校验位，通常为'N'（无校验），'E'（偶校验），'O'（奇校验）
    modbus_t *ctx;      // 指向Modbus上下文的指针，用于Modbus通信
} modbus_serial_t;

typedef struct InclinationSensor_s
{
    float angle_x; // x轴角度值
    float angle_y; // y轴角度值
    float angle_z; // z轴角度值
} InclinationSensor_t;

modbus_serial_t rtuctl;
InclinationSensor_t incsensor;

static struct uloop_timeout g_rtu_timer;

// 转化2字节无符号整数为浮点数，并除以100
float convert_to_signed_float(uint16_t input)
{
    // 将 uint16_t 转换为 int16_t
    int16_t signed_value = (int16_t)input;
    // 除以100得到最终的浮点数
    float final_value = signed_value / 100.0f;
    return final_value;
}

// 定义一个静态函数，用于Modbus RTU定时轮询
static void modbus_rtu_timed_polling(struct uloop_timeout *uloop_t)
{
    int regs = 0;           // 读取到的寄存器数量
    int i = 0;              // 循环计数器
    uint16_t buf[50] = {0}; // 存储读取寄存器数据的缓冲区，初始化为0

    // 读取Modbus寄存器
    // 参数1：Modbus上下文（ctx）
    // 参数2：起始寄存器地址（0x01）
    // 参数3：读取寄存器的数量（0x03）
    // 参数4：用于存储读取数据的缓冲区（buf）
    regs = modbus_read_registers(rtuctl.ctx, 0x01, 0x03, buf);
    if (regs == -1) // 如果读取失败，返回-1
    {
        LOG("%s\n", modbus_strerror(errno)); // 打印错误信息
        return;                              // 退出函数
    }

    LOG("regs  %d  \n", regs); // 打印读取到的寄存器数量

    // 遍历读取到的寄存器数据，并打印每个寄存器的值
    // for (i = 0; i < regs; ++i)
    // {
    //     LOG("<%#x> \n", buf[i]); // 打印寄存器值，以十六进制格式
    // }

    incsensor.angle_x = convert_to_signed_float(buf[0]);
    incsensor.angle_y = convert_to_signed_float(buf[1]);
    incsensor.angle_z = convert_to_signed_float(buf[2]);

    LOG("<%.2f°> \n", incsensor.angle_x); 
    LOG("<%.2f°> \n", incsensor.angle_y); 
    LOG("<%.2f°> \n", incsensor.angle_z); 

end:
    // 重置定时器
    // 设置定时器的回调函数为modbus_rtu_timed_polling
    uloop_t->cb = modbus_rtu_timed_polling;
    // 设置定时器超时时间为1000毫秒（1秒）
    uloop_timeout_set(uloop_t, 1000);
}

// 定义一个静态函数，用于注册Modbus串口
static int modbus_serial_registers(modbus_serial_t *node)
{
    int i = 0;          // 循环计数器
    char port_name[32]; // 串口名称缓冲区

    // 打印调试信息
    LOG("************** modbus_serial_registers start\n");
    LOG("name:%s \n", node->name);               // 打印串口名称
    LOG("serial_type:%d \n", node->serial_type); // 打印串口类型
    LOG("speed:%d \n", node->speed);             // 打印波特率
    LOG("data_bits:%d \n", node->data_bits);     // 打印数据位
    LOG("stop_bits:%d \n", node->stop_bits);     // 打印停止位
    LOG("check_bits:%s \n", node->check_bits);   // 打印校验位

    // 初始化串口名称缓冲区为0
    memset(port_name, 0, 32);
    // 如果串口名称不为空
    if (node->name != NULL)
    {
        // 构建完整的串口设备路径
        sprintf(port_name, "/dev/%s", node->name);
        LOG("name:%s \n", port_name); // 打印完整的串口设备路径
    }
    else
    {
        LOG("name:%s  disenale open\n", port_name); // 打印错误信息
        return -1;                                  // 返回错误
    }

    // 打开Modbus RTU端口
    // 参数1：串口设备路径
    // 参数2：波特率
    // 参数3：校验位（'N'、'E'、'O'）
    // 参数4：数据位
    // 参数5：停止位
    node->ctx = modbus_new_rtu(port_name, node->speed, (char)node->check_bits[0], node->data_bits, node->stop_bits);
    if (node->ctx == NULL) // 如果打开失败
    {
        LOG("Connexion failed: %s\n", modbus_strerror(errno)); // 打印错误信息
        return -1;                                             // 返回错误
    }

    // 设置Modbus调试模式
    modbus_set_debug(node->ctx, TRUE);
    // 设置从设备地址为1
    modbus_set_slave(node->ctx, 1);

    // 设置串口模式
    // 串口类型为1时，设置为RS232
    if (node->serial_type == 1)
    {
        modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS232);
    }
    else // 否则，设置为RS485
    {
        modbus_rtu_set_serial_mode(node->ctx, MODBUS_RTU_RS485);
    }

    // 设置RTS（可选）
    modbus_rtu_set_rts(node->ctx, MODBUS_RTU_RTS_UP);

    // 建立Modbus连接
    if (modbus_connect(node->ctx) == -1) // 如果连接失败
    {
        LOG("Connexion failed: %s\n", modbus_strerror(errno)); // 打印错误信息
        modbus_free(node->ctx);                                // 释放Modbus上下文
        return -1;                                             // 返回错误
    }

    // 设置应答延时（可选）
    // 参数1：秒
    // 参数2：微秒
    modbus_set_response_timeout(node->ctx, 3, 0);

    LOG("************** modbus_serial_registers end\n"); // 打印调试信息，表示函数结束

    return 0; // 成功返回0
}

int main(int argc, char *argv[])
{
    memcpy(rtuctl.name, "ttyS1", 11);
    memcpy(rtuctl.check_bits, "N", 11);

    rtuctl.serial_type = 2;
    rtuctl.speed = 9600;
    rtuctl.data_bits = 8;
    rtuctl.stop_bits = 0;

    modbus_serial_registers(&rtuctl);
    modbus_rtu_timed_polling(&g_rtu_timer);

    uloop_run();
    uloop_done();

    return 0;
}
