cmake_minimum_required(VERSION 3.10)
project(pub)

include_directories(
    src
    src/read_co
    src/read_volume
    src/write_register
    src/read_uci
    src/modbus_init
)

# 源文件
set(SOURCES
    src/pub.c
    src/read_co/read_co.c
    src/read_volume/read_volume.c
    src/write_register/write_register.c
    src/read_uci/read_uci.c
    src/modbus_init/modbus_init.c
)

# 生成可执行文件
add_executable(pub ${SOURCES})

set_target_properties(pub PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}
)

# 链接依赖库
target_link_libraries(pub
    modbus
    uci
    ubox
    ubus
    blobmsg_json
    pthread
    m
)