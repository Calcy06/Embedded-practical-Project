#include "modbus_deal.h"
#include <json-c/json.h>  //头文件要引入

#define COIL_ON 1
#define COIL_OFF 0
/*
 控制线圈开关示例
{
    "devid" :"JDQ"
    "function":5,
    "regStart":0,
    "value":1,
}

 控制 多个线圈开关示例
{
    "devid" :"JDQ"
    "function":15,
    "regStart":0,
    "regNum":8,
    "valueNum":1
    "value_array":[1,0,0,1,0,0,0,0],  //控制 1 4 线圈开2 3 5 6 7 8 线圈关闭
}

 写寄存器示例
{
    "devid" :"JDQ"
    "function":6,
    "regStart":0,
    "value":2,
}

 写多个寄存器示例
{
    "devid" :"JDQ"
    "function":10,
    "regStart":0,
    "regNum":2,
    "valueNum":1,
    "value_array":[12,34,56,78]
}

*/

int modbus_cmd_parse(unsigned char *buffer)
{
    modbus_rtu_node_t *pos, *npos;

    struct json_object *parsed_json;
    struct json_object *function;
    struct json_object *devid;
    struct json_object *regStart;
    struct json_object *value;
    struct json_object *value_array, *valueNum, *regNum;

    int RegStart = 0;
    int Function = 0;
    int ret = 0;

    parsed_json = json_tokener_parse(buffer);
    if (!parsed_json) {
        printf("JSON解析失败\n");
        ret =-1;
        return ret;
    }

    json_object_object_get_ex(parsed_json,"devid", &devid);

    json_object_object_get_ex(parsed_json, "function", &function);

    json_object_object_get_ex(parsed_json, "regStart", &regStart);

    Function = json_object_get_int(function);
    RegStart = json_object_get_int(regStart);
    const char *Devid = json_object_get_string(devid);

    /* 遍历传感器列表获取 */
    list_for_each_entry_safe(pos, npos, &g_modbus_rtu_list, list)
    {
        if (strcmp(pos->sensor_code, Devid) == 0)
        {
            if (pos->ctx)
            {
                modbus_set_slave(pos->ctx, pos->slave);
                break;
            }
        }
    }

    switch (Function)
    {
    case 5:
        // 强置单线圈 强置一个逻辑线圈的通断状态
        json_object_object_get_ex(parsed_json, "value", &value);
        ret = modbus_write_bit(pos->ctx, RegStart, (json_object_get_int(value) == COIL_ON) ? COIL_ON : COIL_OFF);
        break;
    case 6:
        // 预置单寄存器 把具体二进值装入一个保持寄存器
        json_object_object_get_ex(parsed_json, "value", &value);
        ret = modbus_write_register(pos->ctx, RegStart, json_object_get_int(value));
        break;

    case 15:
        // 强置多线圈 强置一串连续逻辑线圈的通断
        json_object_object_get_ex(parsed_json, "regNum", &regNum);
        json_object_object_get_ex(parsed_json, "valueNum", &valueNum);
        json_object_object_get_ex(parsed_json, "value_array", &value_array);
        int value_array_len = json_object_array_length(value_array);
        if (json_object_get_int(valueNum) != value_array_len && json_object_get_int(valueNum) != json_object_get_int(regNum) )
        {
            ret = 1;
            goto end; // 字节丢失或者 线圈数量的控制与控制字节数不符合
        }

        uint8_t *src = (uint8_t *)malloc(value_array_len *sizeof(uint8_t));
        memset(src,value_array_len,0);
        if (!src)
        {
            printf("malloc error\n");
            ret = 1;
            goto end;
        }
        printf("value_array_len %d\n",value_array_len);

        for (int i = 0; i < value_array_len; i++)
        {
            struct json_object *val = json_object_array_get_idx(value_array, i);
            src[i] = (uint8_t)json_object_get_int(val);          
        }
        ret = modbus_write_bits(pos->ctx, RegStart, json_object_get_int(regNum),src);
        free(src);
        src = NULL;
        break;
    case 16:
        json_object_object_get_ex(parsed_json, "regNum", &regNum);
        json_object_object_get_ex(parsed_json, "valueNum", &valueNum);
        json_object_object_get_ex(parsed_json, "value_array", &value_array);
        int fun_16_value_array_len = json_object_array_length(value_array);
        if (json_object_get_int(valueNum) != fun_16_value_array_len && json_object_get_int(valueNum) != json_object_get_int(regNum) )
        {
            ret = 1;
            goto end; // 字节丢失或者 线圈数量的控制与控制字节数不符合
        }

        uint8_t *fun_16_src = (uint8_t *)malloc(fun_16_value_array_len *sizeof(uint8_t));
        memset(fun_16_src,fun_16_value_array_len,0);
        if (!fun_16_src)
        {
            printf("malloc error\n");
            ret = 1;
            goto end;
        }
        printf("value_array_len %d\n",fun_16_value_array_len);

        for (int i = 0; i < fun_16_value_array_len; i++)
        {
            struct json_object *val = json_object_array_get_idx(value_array, i);
            fun_16_src[i] = (uint8_t)json_object_get_int(val);          
        }
        ret = modbus_write_registers(pos->ctx, RegStart, json_object_get_int(regNum),fun_16_src);
        free(fun_16_src);
        fun_16_src = NULL;
        break;

        //  int modbus_write_registers(modbus_t *ctx, int addr, int nb, const uint16_t *src) 解析的时候注意申请 u16
        

    case 23:
        // 报告从机标识 可使主机判断编址从机的类型及该从机运行指示灯的状态
        //  int modbus_write_and_read_registers(modbus_t *ctx,int write_addr, int write_nb, const uint16_t *src,int read_addr, int read_nb, uint16_t *dest)
        ret = 1;
        break;
    default:
        // 这边统一回复 暂不支持
        ret = 1;
        break;
    }

end:
    json_object_put(parsed_json);
    return ret;
}


