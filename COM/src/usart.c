#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <time.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <assert.h>

#include <libubox/uloop.h>
#include <libubox/usock.h>


struct uloop_fd g_com_dev;
struct uloop_timeout g_time_send;

/* 存储串口号 */
char serial[50] = {0};

/* 存储波特率 */
uint32_t bound = 0;

static void set_parity(struct termios *opt, char parity)
{
    switch (parity)
    {
    case 'N': /* no parity check */
        opt->c_cflag &= ~PARENB;
        break;
    case 'E': /* even */
        opt->c_cflag |= PARENB;
        opt->c_cflag &= ~PARODD;
        break;
    case 'O': /* odd */
        opt->c_cflag |= PARENB;
        opt->c_cflag |= ~PARODD;
        break;
    default: /* no parity check */
        opt->c_cflag &= ~PARENB;
        break;
    }
}

static void set_data_bit(struct termios *opt, unsigned int databit)
{
    opt->c_cflag &= ~CSIZE;
    switch (databit)
    {
    case 8:
        opt->c_cflag |= CS8;
        break;
    case 7:
        opt->c_cflag |= CS7;
        break;
    case 6:
        opt->c_cflag |= CS6;
        break;
    case 5:
        opt->c_cflag |= CS5;
        break;
    default:
        opt->c_cflag |= CS8;
        break;
    }
}

static uint32_t com_set_speed(uint32_t baudrate)
{
    uint32_t speed;
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
        speed = 0;
        break;
    }
    return speed;

}

static void set_stopbit(struct termios *opt, const char stopbit)
{
    if (1 == stopbit)
    {
        opt->c_cflag &= ~CSTOPB; /* 1 stop bit */
    }
    else if (2 == stopbit)
    {
        opt->c_cflag |= CSTOPB; /* 2 stop bits */
    }
    else
    {
        opt->c_cflag &= ~CSTOPB; /* 1 stop bit */
    }
}

int com_dev_init()
{
    struct termios opt;
    int fd;
    int32_t speed;
    uint32_t databit;
    char dev_name[] = "/dev/ttyS3";

    /*打开串口  O_NDELAY */
    fd = open(serial, O_RDWR | O_NOCTTY | O_NDELAY);
    if (fd < 0)
    {
        printf("Error: open() %s failed\n", serial);
        return -1;
    }

    /* 串口参数设置 */
    tcgetattr(fd, &opt);
    opt.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG); //设置为原始模式
    opt.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    opt.c_oflag &= ~OPOST;

    //数据长度位
    set_data_bit(&opt, 8);

    //校验
    set_parity(&opt, 'N');

    //停止位
    set_stopbit(&opt, 1);

    //波特率 9600
    speed = com_set_speed(bound);
    cfsetispeed(&opt, speed); //设置输入波特率
    cfsetospeed(&opt, speed); //设置输出波特率

    tcsetattr(fd, TCSANOW, &opt); //设置立即生效

    return fd;
}

u_char rxbuffer[2014];
int rs485_receive_test(struct uloop_fd *uloop_fd, unsigned int events)
{
    int rev_ele;
    int i;

    rev_ele = read(uloop_fd->fd, rxbuffer, sizeof(rxbuffer) - 1);

    printf("read data begin %d buffer %s \n", rev_ele,rxbuffer);
   
}

u_int32_t count;
void rs485_send_test(struct uloop_timeout *t)
{
    count++;
    char buf[] = "helloworld";
    write(g_com_dev.fd, buf, sizeof(buf));
    printf("send buf count %d (%s)\n",count,buf);

    uloop_timeout_set(t, 100);
}

int main(int argc, char **argv)
{   
    if (argc != 3)
    {
        fprintf(stderr,"Please input correct serial and baudrate.\n");
        return -1;
    }

    if(strlen(argv[1]) <=5)
    {
        fprintf(stderr,"Please input correct serial. Eg:/dev/ttySx \n");
        return -1;
    }

    if(!com_set_speed(atoi(argv[2])))
    {
        fprintf(stderr,"Please input correct baudrate. Eg:115200 \n");
        return -1;
    }

    strcpy(serial,argv[1]);
    bound = atoi(argv[2]);

    printf("serial %s\n",serial);
    printf("baudrate %d\n",bound);


    uloop_init();

    //串口
    g_com_dev.fd = com_dev_init();
    g_com_dev.cb = rs485_receive_test;
    uloop_fd_add(&g_com_dev, ULOOP_READ);
    printf("uart init \n");

    //定时器
    g_time_send.pending = false;
    g_time_send.cb = rs485_send_test;
    uloop_timeout_set(&g_time_send, 1000);
    printf("状态监测 init \n");

    uloop_run();
    uloop_done();
}