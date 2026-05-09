# modbus_rtu



## 下载&编译

```
git clone http://172.16.16.186/zr-hardware-group1/zr-hardware-rk3588j/app/modbus_rtu.git
cd  modbus_rtu 
mkdir build && cd build
cmake .. 
make && make install

```




## 配置文件解释

-[ ] [增加传感器配置]

```
config modbus_rtu 
	option serial		'ttyXRUSB3'        #占用串口
	option slave		'1'            #从机地址
	option r_enable		'1'            #读使能
	option period       '20'			   #采集周期
	option sensor_code		'relay'  #传感器编码
	list   func_mes '1.0.8.RelayControl'      #功能码 寄存器地址(十进制)  寄存器数量  数据处理方法(在lua脚本中会体现)    


```

-[ ] [增加总线配置]

```
config serial 
	option busname 'ttyXRUSB3'
	option enable		'1'
	option serial_type	'2'
	option speed 		'38400'
	option data_bits	'8'
	option stop_bits	'1'
	option check_bits	'N'

```


## UBUS接口调试
```
控制继电器 第1个口 开
ubus call modbus_rtu write '{ "devid" :"relay", "function":5,"regStart":0,"value":1}'

控制继电器 8个口 前四个开 后四个关
ubus call modbus_rtu write '{"devid":"relay","function":15,"regStart":0,"regNum":8,"valueNum":8,"value_array":[1,1,1,1,0,0,0,0]}'

控制脉冲计数器清空第4路脉冲
ubus call modbus_rtu write '{"devid":"FT—PFC-04","function":16,"regStart":54,"regNum":2,"valueNum":4,"value_array":[0,0,0,0]}'

控制声光报警器动作
ubus call modbus_rtu write '{"devid":"BJQ","function":6,"regStart":1,"regNum":1,"value":1}'

```


## Web配置


