#ifndef __DATA_DEAL_H__
#define __DATA_DEAL_H__

/**
 * @brief  IEEE-754标准数据处理函数（32位）
 * @param  number  需要处理十六进制数据
 * @return float类型的数据
 */
float IEEE_754Hex2Float(int number);

uint64_t Get_time_stam(void);

#endif /*__DATA_DEAL_H__*/