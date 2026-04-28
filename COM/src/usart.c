#include <stdio.h>
// Unix标准头文件，提供 close、read、write、usleep 等系统调用
#include <unistd.h>
// 标准库头文件，提供 malloc、free、atoi 等通用函数
#include <stdlib.h>
// 字符串操作头文件，提供 strcpy、strlen、memset 等字符串/内存操作
#include <string.h>
// 文件控制头文件，提供 open 等文件打开相关的宏和函数
#include <fcntl.h>
// 字符类型判断头文件，提供 isdigit、isalpha 等判断字符类型的函数
#include <ctype.h>
// 时间头文件，提供 time、clock 等时间相关函数
#include <time.h>
// 错误码头文件，提供 errno 错误码变量，用于获取系统调用错误原因
#include <errno.h>
// 文件状态头文件，提供文件属性相关结构体和函数
#include <sys/stat.h>
// 系统数据类型头文件，定义系统使用的基本数据类型
#include <sys/types.h>
// 重复包含，作用同上
#include <sys/stat.h>
// 串口配置头文件，Linux下专门用于配置串口参数（波特率、数据位等）
#include <termios.h>
// 断言头文件，用于程序调试，判断条件是否成立
#include <assert.h>

// libubox 事件循环头文件，提供 uloop 事件驱动框架（核心：监听文件描述符、定时器）
#include <libubox/uloop.h>
// libubox 套接字头文件，提供网络/串口套接字相关工具函数
#include <libubox/usock.h>

// 全局变量：libubox 事件监听结构体，用于监听串口文件描述符的可读事件
struct uloop_fd g_com_dev;
// 全局变量：libubox 定时器结构体，用于定时发送串口数据
struct uloop_timeout g_time_send;

/* 存储串口号
 * 全局数组：存放串口设备路径，例如 /dev/ttyS0、/dev/ttyS3
 * 长度50，足够存放绝大多数Linux串口设备名
 */
char serial[50] = {0};

/* 存储波特率
 * 全局变量：存放串口波特率，例如 9600、115200
 * uint32_t：无符号32位整型，保证数据范围足够
 */
uint32_t bound = 0;

/********************************************************************
* 函数功能：设置串口奇偶校验位
* 传入参数：
*   struct termios *opt ：串口配置结构体指针，存放所有串口参数
*   char parity         ：校验位类型，'N'无校验 / 'E'偶校验 / 'O'奇校验
* 返回值：无
********************************************************************/
// |= 打开
// &= ~ 关闭
static void set_parity(struct termios *opt, char parity)
{
    // 根据传入的校验位类型选择配置
    switch (parity)
    {
    case 'N': /* no parity check 无校验 */
        opt->c_cflag &= ~PARENB;  // 关闭奇偶校验功能
        break;
    case 'E': /* even 偶校验 */
        opt->c_cflag |= PARENB;  // 开启奇偶校验功能
        opt->c_cflag &= ~PARODD; // 设置为偶校验
        break;
    case 'O': /* odd 奇校验 */
        opt->c_cflag |= PARENB;  // 开启奇偶校验功能
        opt->c_cflag |= ~PARODD; // 设置为奇校验
        break;
    default: /* no parity check 默认：无校验 */
        opt->c_cflag &= ~PARENB;
        break;
    }
}

/********************************************************************
* 函数功能：设置串口数据位
* 传入参数：
*   struct termios *opt   ：串口配置结构体指针
*   unsigned int databit  ：数据位位数，支持 5/6/7/8 位
* 返回值：无
********************************************************************/
static void set_data_bit(struct termios *opt, unsigned int databit)
{
    opt->c_cflag &= ~CSIZE;  // 先清空原有的数据位配置
    // 根据传入的数据位选择配置
    switch (databit)
    {
    case 8:
        opt->c_cflag |= CS8; // 8位数据位
        break;
    case 7:
        opt->c_cflag |= CS7; // 7位数据位
        break;
    case 6:
        opt->c_cflag |= CS6; // 6位数据位
        break;
    case 5:
        opt->c_cflag |= CS5; // 5位数据位
        break;
    default:
        opt->c_cflag |= CS8; // 默认：8位数据位
        break;
    }
}

/********************************************************************
* 函数功能：将数字波特率转换为系统宏定义（Linux串口专用）
* 传入参数：
*   uint32_t baudrate ：数字波特率，例如 9600、115200
* 返回值：对应的系统波特率宏定义，失败返回0
********************************************************************/
static uint32_t com_set_speed(uint32_t baudrate)
{
    uint32_t speed;  // 存放转换后的系统宏
    // 匹配常用波特率
    switch (baudrate)
    {
    case 300:
        speed = B300;
        break;
    case 1200:
        speed = B1200;
        break;
    case 2400:
        speed = B2400;
        break;
    case 4800:
        speed = B4800;
        break;
    case 9600:
        speed = B9600;
        break;
    case 19200:
        speed = B19200;
        break;
    case 38400:
        speed = B38400;
        break;
    case 115200:
        speed = B115200;
        break;
    case 460800:
        speed = B460800;
        break;
    default:
        speed = 0;  // 不支持的波特率，返回0
        break;
    }
    return speed;
}

/********************************************************************
* 函数功能：设置串口停止位
* 传入参数：
*   struct termios *opt ：串口配置结构体指针
*   const char stopbit  ：停止位位数，1位 / 2位
* 返回值：无
********************************************************************/
static void set_stopbit(struct termios *opt, const char stopbit)
{
    if (1 == stopbit)
    {
        opt->c_cflag &= ~CSTOPB; /* 1 stop bit 1位停止位 */
    }
    else if (2 == stopbit)
    {
        opt->c_cflag |= CSTOPB; /* 2 stop bits 2位停止位 */
    }
    else
    {
        opt->c_cflag &= ~CSTOPB; /* 默认：1位停止位 */
    }
}

/********************************************************************
* 函数功能：串口初始化函数（打开串口+配置参数）
* 传入参数：无（使用全局变量 serial、bound）
* 返回值：成功返回串口文件描述符fd，失败返回-1
********************************************************************/
int com_dev_init()
{
    struct termios opt;  // 串口配置结构体，存放所有参数
    int fd;              // 文件描述符，Linux中代表打开的设备/文件
    int32_t speed;       // 转换后的系统波特率
    uint32_t databit;    // 数据位变量
    char dev_name[] = "/dev/ttyS3"; // 示例串口名，代码中未使用

    /*打开串口
     * O_RDWR    ：读写模式打开
     * O_NOCTTY  ：不把串口作为控制终端
     * O_NDELAY  ：非阻塞模式打开
     */
    fd = open(serial, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)  // 打开失败，返回负数
    {
        printf("Error: open() %s failed\n", serial);
        return -1;
    }

    /* 串口参数设置：设置为原始模式（适合串口透传数据）*/
    tcgetattr(fd, &opt);  // 获取当前串口配置
    // 关闭规范模式、回显、信号字符，设置为原始模式
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);
    // 关闭输入流的特殊处理
    opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    // 关闭输出流的特殊处理
    opt.c_oflag &= ~OPOST;

    //数据长度位：设置为8位
    set_data_bit(&opt, 8);

    //校验位：设置为无校验
    set_parity(&opt, 'N');

    //停止位：设置为1位
    set_stopbit(&opt, 1);

    //波特率：使用全局变量 bound
    speed = com_set_speed(bound);
    cfsetispeed(&opt, speed); //设置输入波特率
    cfsetospeed(&opt, speed); //设置输出波特率

    tcsetattr(fd, TCSANOW, &opt); //配置立即生效

    return fd;  // 返回串口文件描述符
}

// 全局接收缓冲区：存放串口收到的数据，大小2014字节
u_char rxbuffer[2014];//rxbuffrt(接收缓冲区)

/********************************************************************
* 函数功能：串口数据接收回调函数（libubox 监听到串口可读时自动调用）
* 传入参数：
*   struct uloop_fd *uloop_fd ：libubox 事件结构体，包含串口fd
*   unsigned int events       ：触发的事件类型（可读/可写）
* 返回值：int（libubox 回调固定格式）
********************************************************************/
int rs485_receive_test(struct uloop_fd *uloop_fd, unsigned int events)
{
    int rev_ele;  // 存放实际读取到的字节数
    int i;        // 循环变量（未使用）

    // 从串口读取数据，存入缓冲区，最大读取2013字节
    rev_ele = read(uloop_fd->fd, rxbuffer, sizeof(rxbuffer) - 1);

    // 打印读取到的字节数和数据
    printf("read data begin %d buffer %s \n", rev_ele,rxbuffer);
   
}

// 全局变量：发送数据计数，记录发送次数
u_int32_t count;

/********************************************************************
* 函数功能：定时器回调函数，定时发送串口数据
* 传入参数：
*   struct uloop_timeout *t ：libubox 定时器结构体
* 返回值：无
********************************************************************/
void rs485_send_test(struct uloop_timeout *t)
{
    count++;  // 发送次数+1
    char buf[] = "helloworld"; // 要发送的数据
    // 向串口写入数据
    write(g_com_dev.fd, buf, sizeof(buf));
    // 打印发送信息
    printf("send buf count %d (%s)\n",count,buf);

    uloop_timeout_set(t, 100); // 重新设置定时器，100ms后再次触发
}

/********************************************************************
* 函数功能：主函数，程序入口
* 传入参数：
*   int argc      ：命令行参数个数
*   char **argv   ：命令行参数数组
* 返回值：0正常退出，-1异常退出
********************************************************************/
int main(int argc, char **argv)
{   
    // 判断命令行参数是否为3个（程序名 + 串口路径 + 波特率）
    if (argc != 3)
    {
        fprintf(stderr,"Please input correct serial and baudrate.\n");
        return -1;
    }

    // 判断串口路径长度是否合法（至少大于5，例如 /dev/ttyS0）
    if(strlen(argv[1]) <=5)
    {
        fprintf(stderr,"Please input correct serial. Eg:/dev/ttySx \n");
        return -1;
    }

    // 判断波特率是否合法
    if(!com_set_speed(atoi(argv[2])))
    {
        fprintf(stderr,"Please input correct baudrate. Eg:115200 \n");
        return -1;
    }

    // 复制命令行传入的串口路径到全局变量
    strcpy(serial,argv[1]);
    // 转换命令行波特率为数字，存入全局变量
    bound = atoi(argv[2]);

    // 打印配置信息
    printf("serial %s\n",serial);
    printf("baudrate %d\n",bound);

    // 初始化 libubox 事件循环框架
    uloop_init();

    //串口初始化：获取串口fd
    g_com_dev.fd = com_dev_init();
    // 绑定串口可读事件的回调函数
    g_com_dev.cb = rs485_receive_test;
    // 将串口添加到事件监听，监听可读事件
    uloop_fd_add(&g_com_dev, ULOOP_READ);
    printf("uart init \n");

    //定时器初始化
    g_time_send.pending = false;
    // 绑定定时器回调函数
    g_time_send.cb = rs485_send_test;
    // 设置定时器，1000ms后第一次触发
    uloop_timeout_set(&g_time_send, 1000);
    printf("状态监测 init \n");

    uloop_run();  // 启动事件循环（死循环，监听事件）
    uloop_done(); // 退出事件循环，释放资源

    return 0;
}