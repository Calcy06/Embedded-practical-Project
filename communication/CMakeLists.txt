cmake_minimum_required(VERSION 3.5)
project(uci_usock_uloop C)

include_directories(
    main
    net
    read
)

add_executable(com
    src/main/main.c
    src/net/net.c
    src/read/read.c
)

target_link_libraries(com PRIVATE ubox uci)