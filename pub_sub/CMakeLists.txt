cmake_minimum_required(VERSION 3.5)
project(pub_sub C)

# 所有头文件目录
include_directories(
    src/free
    src/modbus_init
    src/read_gg
    src/read_sg
    src/read_uci
    src/write_register
)

# 可执行文件：pub
add_executable(pub
    src/pub.c
    src/free/free.c
    src/modbus_init/modbus_init.c
    src/read_gg/read_gg.c
    src/read_sg/read_sg.c
    src/read_uci/read_uci.c
    src/write_register/write_register.c
)

# 可执行文件：sub
add_executable(sub
    src/sub.c
    src/free/free.c
    src/modbus_init/modbus_init.c
    src/read_gg/read_gg.c
    src/read_sg/read_sg.c
    src/read_uci/read_uci.c
    src/write_register/write_register.c
)

# 链接依赖库（ubus/ubox/uci/modbus）
target_link_libraries(pub PRIVATE ubus ubox uci blobmsg_json modbus)
target_link_libraries(sub PRIVATE ubus ubox uci blobmsg_json modbus)
