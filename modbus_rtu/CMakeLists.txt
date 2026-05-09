cmake_minimum_required(VERSION 3.5)

project(modbus_rtu C)


set(MODBUS_TOP_LEVEL_DIRECTORY "${CMAKE_CURRENT_LIST_DIR}")


function(target_sources_modbus_rtu target)
    target_sources(
        ${target}
        PRIVATE ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg/cfg.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/tool/data_deal.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/modbus_deal/modbus_deal.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus/ubus.c
                ${MODBUS_TOP_LEVEL_DIRECTORY}/src/modbus_rtu.c
                
    )

    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/ubus)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/cfg)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/tool)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src/modbus_deal)
    target_include_directories(${target} PUBLIC ${MODBUS_TOP_LEVEL_DIRECTORY}/src)

endfunction()


add_compile_options( -g)


add_executable(modbus_rtu)

target_link_libraries(modbus_rtu PRIVATE -lblobmsg_json -lubox -luci -lubus -ldebug -ljson-c -ldl  -lmodbus -lm -llua   -ldl)


target_sources_modbus_rtu(modbus_rtu)


#拷贝启动文件
install(
    TARGETS modbus_rtu
    DESTINATION /usr/local/sbin
)

#拷贝开机启动文件
install(
    FILES  ${MODBUS_TOP_LEVEL_DIRECTORY}/etc/init.d/modbus_rtu.service
    DESTINATION /lib/systemd/system
    PERMISSIONS 
        OWNER_READ OWNER_WRITE   # 所有者：读写
        GROUP_READ GROUP_WRITE   # 用户组：读写
        WORLD_READ WORLD_WRITE   # 其他用户：读写
   
)

#拷贝lua解析脚本
install(
    FILES  ${MODBUS_TOP_LEVEL_DIRECTORY}/usr/local/lib/lua/modbus_rtu/modbus_data_deal
    DESTINATION /usr/local/lib/lua/modbus_rtu/
    PERMISSIONS 
        OWNER_READ OWNER_WRITE   # 所有者：读写
        GROUP_READ GROUP_WRITE   # 用户组：读写
        WORLD_READ WORLD_WRITE   # 其他用户：读写
   
)

#拷贝配置文件
install(
    DIRECTORY ${MODBUS_TOP_LEVEL_DIRECTORY}/etc/config/
    DESTINATION /etc/config
    FILES_MATCHING PATTERN "*"
   
)



add_custom_target(clean-install

    COMMAND ${CMAKE_COMMAND} -E remove /lib/systemd/system/modbus_rtu.service
    COMMAND ${CMAKE_COMMAND} -E remove /usr/local/lib/lua/modbus_rtu/modbus_data_deal
    COMMAND ${CMAKE_COMMAND} -E remove /etc/config/modbus_rtu
    COMMAND ${CMAKE_COMMAND} -E remove /etc/config/modbus_serial
    COMMAND ${CMAKE_COMMAND} -E remove /usr/local/sbin/modbus_rtu

    COMMENT "Cleaning up install files. "
)

add_custom_target(cleaninstall DEPENDS clean-install)
