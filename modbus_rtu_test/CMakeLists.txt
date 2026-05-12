cmake_minimum_required(VERSION 3.5)

project(modbus_rtu C)

# 定义项目根目录路径（当前CMake文件所在目录）
set(MODBUS_TOP_LEVEL_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

# ==============================================
# 函数：给 target 添加通用源码（两个程序共用的代码）
# 包含：cfg.c、ubus.c（公共库文件）
# ==============================================
function(target_sources_common target)
    target_sources(
        ${target}
        PRIVATE ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg/cfg.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus/ubus.c
    )

    # 添加头文件搜索路径
    target_include_directories(${target}
        PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus
        PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg
        PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src
    )
endfunction()

# 全局编译选项：-g 生成调试信息
add_compile_options(-g)

# ==============================================
# 生成主程序：modbus_rtu
# 使用：modbus_rtu.c + 公共文件 cfg.c + ubus.c
# ==============================================
add_executable(modbus_rtu
    ${MODBUS_TOP_LEVEL_DIRECTORY}/src/modbus_rtu.c
)
target_sources_common(modbus_rtu)
target_link_libraries(modbus_rtu PRIVATE
    -lblobmsg_json
    -lubox
    -luci
    -lubus
    -ljson-c
    -lmodbus
)

# ==============================================
# 生成子程序：sub
# 使用：sub.c + 公共文件 cfg.c + ubus.c
# ==============================================
add_executable(sub
    ${MODBUS_TOP_LEVEL_DIRECTORY}/src/sub.c
)
target_sources_common(sub)
target_link_libraries(sub PRIVATE
    -lblobmsg_json
    -lubox
    -luci
    -lubus
    -ljson-c
    -lmodbus
)