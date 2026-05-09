cmake_minimum_required(VERSION 3.5)

project(modbus_rtu C)


set(MODBUS_TOP_LEVEL_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")


function(target_sources_modbus_rtu target)
    target_sources(
        ${target}
        PRIVATE ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg/cfg.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus/ubus.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/modbus_rtu.c
                
    )

    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src)

endfunction()


add_compile_options( -g)


add_executable(modbus_rtu)

target_link_libraries(modbus_rtu PRIVATE -lblobmsg_json -lubox -luci -lubus  -ljson-c -lmodbus  )


target_sources_modbus_rtu(modbus_rtu)


