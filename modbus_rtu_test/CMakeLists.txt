# 指定 CMake 最低版本要求，低于 3.5 版本的 CMake 会报错
cmake_minimum_required(VERSION 3.5)

# 定义项目名称为 modbus_rtu，使用 C 语言编译
project(modbus_rtu C)

# 定义变量：MODBUS_TOP_LEVEL_DIRECTORY = 当前 CMakeLists.txt 所在目录
# 用来方便后续引用源码路径，避免重复写长路径
set(MODBUS_TOP_LEVEL_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")

# 定义一个函数：target_sources_modbus_rtu
# 作用：给指定目标（target）添加源码和头文件路径
function(target_sources_modbus_rtu target)

    # 给 target 添加 源文件（PRIVATE 表示仅内部使用）
    target_sources(
        ${target}
        PRIVATE ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg/cfg.c          # 配置文件解析
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus/ubus.c        # ubus 通信相关
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/modbus_rtu.c       # 主程序入口
    )

    # 给 target 添加 头文件搜索路径
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src)

endfunction()

# 添加编译选项：-g 表示生成调试信息（方便 GDB 调试）
add_compile_options( -g )

# 定义生成可执行文件，名字叫 modbus_rtu
add_executable(modbus_rtu)

# 给可执行文件链接外部库（OpenWRT 标准库 + modbus 库）
target_link_libraries(modbus_rtu PRIVATE
    -lblobmsg_json
    -lubox
    -luci
    -lubus
    -ljson-c
    -lmodbus
)

# 调用上面定义的函数，给 modbus_rtu 程序添加源码与头文件路径
target_sources_modbus_rtu(modbus_rtu)