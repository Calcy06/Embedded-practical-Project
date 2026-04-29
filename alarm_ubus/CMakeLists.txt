cmake_minimum_required(VERSION 3.5)
project(alarm_ubus C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_FLAGS "-Wall -O2")

# 头文件路径
include_directories(
    cfg
    src/modbus
    src/ubus
)

# 源文件
add_executable(alarm_ubus
    src/main/main.c
    src/modbus/modbus.c
    src/ubus/ubus.c
)

# 链接库
target_link_libraries(alarm_ubus
    PRIVATE
        modbus
        ubus
        ubox
        blobmsg_json
)