 嵌入式 Modbus TCP 转发客户端（UBUS + UCI + TCP 透传）
 基本介绍
该软件是基于 OpenWrt 平台开发的嵌入式 C 语言项目，依托 libubox 事件驱动框架（uloop/usock）、UBUS 进程间通信机制与 UCI 配置系统，实现UBUS 事件监听 + Modbus 定时主动调用 + TCP 透传转发三合一功能，可将嵌入式设备的 UBUS 事件、Modbus 数据实时转发至电脑端 TCP 服务器，完成设备数据上送与远程控制。

 软件特色
项目采用高内聚、低耦合的模块化设计，专为嵌入式低资源环境优化，具备以下核心特点：
- 模块化拆分：代码按功能拆分为 UCI 配置读取、Modbus 初始化、UBUS 事件订阅、TCP 长连接重连、内存安全管理五大独立模块，逻辑解耦、易维护
- UCI 动态配置：支持通过配置文件修改 TCP 服务器地址、串口参数、Modbus 配置，无需编译直接生效
- 事件驱动架构：基于 uloop 事件循环，同时处理 UBUS 事件、TCP 收发、定时任务，全程非阻塞、无多线程
- UBUS 双事件订阅：同时监听 `gg`/`sg` 两个 UBUS 对象事件，统一回调处理，支持灵活扩展事件类型
- Modbus 定时触发：内置 5 秒周期定时器，自动调用 Modbus `write_register` 接口，实现周期性数据上报
- TCP 长连接保障：内置自动重连机制，网络断开后自动恢复，保证数据传输稳定性
- 内存/资源安全：严格遵循内存分配判空、动态资源释放、UBUS 资源注销规范，无内存泄漏、无野指针崩溃

 主要功能
软件满足嵌入式设备数据采集、事件上报、远程透传、定时控制四大核心需求：
- UCI 配置自动加载：启动时读取服务器 IP、端口、Modbus 配置，一键完成初始化
- TCP 客户端透传：与电脑端 TCP 服务器建立长连接，实现 UBUS/Modbus 数据实时转发
- UBUS 事件监听转发：订阅 `gg`/`sg` 双对象事件，事件触发后自动转为 JSON 格式发送至 TCP 服务器
- Modbus 定时主动调用：5 秒周期性执行 Modbus 写寄存器操作，支持自定义周期与功能码
- 自动重连机制：TCP 断开后自动重试连接，保证通信不中断
- 安全退出机制：程序退出时自动释放套接字、注销 UBUS 订阅、释放内存，无残留资源

---

 Embedded Modbus TCP Forwarding Client (UBUS + UCI)
 Basic Introduction
This software is an embedded C language project based on the OpenWrt platform. It relies on the libubox event-driven framework (uloop/usock), UBUS inter-process communication mechanism, and UCI configuration system to implement three-in-one functions: UBUS event monitoring, Modbus timed active calling, and TCP transparent transmission forwarding. It can real-time forward UBUS events and Modbus data of embedded devices to the PC-side TCP server for device data reporting and remote control.

 Software Features
The project adopts a highly cohesive, low-coupling modular design optimized for embedded low-resource environments, with the following core features:
- Modular Separation: The code is split into five independent modules: UCI configuration reading, Modbus initialization, UBUS event subscription, TCP long connection reconnection, and memory security management, with decoupled logic and easy maintenance.
- UCI Dynamic Configuration: Supports modifying TCP server address, serial port parameters, and Modbus configurations through configuration files without recompiling.
- Event-Driven Architecture: Based on the uloop event loop, it simultaneously handles UBUS events, TCP sending/receiving, and timed tasks, fully non-blocking without multi-threading.
- UBUS Dual Event Subscription: Monitors `gg`/`sg` two UBUS object events at the same time, processed by a unified callback, supporting flexible expansion of event types.
- Modbus Timed Trigger: Built-in 5-second cycle timer to automatically call the Modbus `write_register` interface for periodic data reporting.
- TCP Long Connection Guarantee: Built-in automatic reconnection mechanism, automatically recovers after network disconnection to ensure stable data transmission.
- Memory & Resource Security: Strictly follows memory allocation null check, dynamic resource release, and UBUS resource logout specifications, no memory leaks or wild pointer crashes.

 Main Functions
The software meets the four core requirements of embedded devices: data collection, event reporting, remote transparent transmission, and timed control:
- Automatic UCI Configuration Loading: Reads server IP, port, and Modbus configurations at startup to complete initialization with one click.
- TCP Client Transparent Transmission: Establishes a long connection with the PC-side TCP server to realize real-time forwarding of UBUS/Modbus data.
- UBUS Event Monitoring & Forwarding: Subscribes to `gg`/`sg` dual-object events, automatically converts to JSON format and sends to the TCP server when events are triggered.
- Modbus Timed Active Call: Executes Modbus write register operation periodically every 5 seconds, supporting custom cycles and function codes.
- Automatic Reconnection Mechanism: Automatically retries the connection after TCP disconnection to ensure uninterrupted communication.
- Safe Exit Mechanism: Automatically releases sockets, logs out UBUS subscriptions, and frees memory when the program exits, no residual resources.

---

 补充：编译与运行说明
 项目结构
```
/root/test/modbus_tcp/
├── CMakeLists.txt    编译配置脚本
├── main.c            主程序入口（事件循环+UBUS初始化）
├── modbus_init.c     Modbus 串口初始化模块
├── uci_tcp.c         TCP 连接 + 重连管理
├── read_uci.c        UCI 配置读取
├── free.c            内存/节点资源释放
├── sub.c             UBUS 订阅 + 事件转发
└── common.h          通用头文件
```

 1. 配置文件准备
创建 UCI 配置文件，配置 TCP 服务器地址与端口：
```bash
cat > /etc/config/modbus_client << EOF
config client 'main'
    option ip '192.168.4.94'
    option port '61000'
EOF
```

 2. 编译命令
```bash
cd /root/test/modbus_tcp
rm -rf build && mkdir build && cd build
cmake ..
make -j4
```

 3. 运行前准备
电脑端打开 TCP 网络调试助手，创建 TCP 服务器，IP/端口与配置文件保持一致。

 4. 运行程序
```bash
./modbus_tcp
```

 5. 功能验证
- 程序启动后自动连接 TCP 服务器
- 每 5 秒主动执行一次 Modbus 写寄存器操作
- `gg`/`sg` 任意 UBUS 事件触发，数据会实时转发到电脑端
- 程序退出自动释放所有资源，无报错、无崩溃
