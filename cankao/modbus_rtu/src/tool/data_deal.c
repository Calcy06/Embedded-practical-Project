#include <time.h>
#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <termios.h>
#include <assert.h>
#include <stdint.h>

#include "data_deal.h"

#define HEX_LEN_ERROR -1
#define NaN_ERROR -2

uint64_t Get_time_stam(void)
{
	struct timeval time_iot;
	gettimeofday(&time_iot, NULL);
	return (uint64_t)time_iot.tv_sec * 1000 + (uint64_t)time_iot.tv_usec / 1000;
}

float IEEE_754Hex2Float(int number)
{
	int sign = (number & 0x80000000) ? -1 : 1;	  // 三目运算符
	int exponent = ((number >> 23) & 0xff) - 127; // 先右移操作，再按位与计算，出来结果是30到23位对应的e

	float mantissa = 1 + ((float)(number & 0x7fffff) / 0x7fffff); // 将22~0转化为10进制，得到对应的x

	if (exponent == 128 && mantissa == 1.0)
	{
		return NaN_ERROR;
	}

	return sign * mantissa * pow(2, exponent);
}
