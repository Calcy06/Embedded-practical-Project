嵌入式TCP双向对话客户端
 
 
 
基本介绍
 
该软件是一个基于OpenWrt平台的嵌入式C语言项目，采用libubox的 usock / uloop 库实现TCP通信与事件驱动，结合UCI配置系统，从零开始构建了一个可与电脑网络调试工具双向对话的客户端程序。
 
软件特色
 
软件采用模块化设计，适配嵌入式开发场景，具备以下特点：
 
- 模块化架构：代码拆分为配置读取、网络通信、键盘输入三个独立模块，结构清晰，便于维护与扩展
- UCI配置管理：通过UCI配置文件读取服务器IP与端口，无需修改代码即可快速切换目标服务器
- 事件驱动IO：基于uloop事件循环，同时处理网络接收与键盘输入，全程非阻塞，不依赖多线程
- 极简实现：仅依赖 libubox 与 uci 库，无冗余依赖，适配资源受限的嵌入式设备
- 轻量对话：支持与电脑网络调试工具进行双向文本对话，输入 exit 即可安全退出程序
 
主要功能
 
软件核心功能可满足嵌入式设备与电脑的网络调试、双向通信需求：
 
- 配置自动加载：启动时自动读取UCI配置文件中的服务器IP与端口，完成初始化
- TCP客户端连接：基于usock实现稳定的TCP客户端，可与电脑端TCP服务器建立连接
- 键盘输入发送：支持通过标准输入发送文本数据至服务器，实现主动通信
- 服务器数据接收：实时监听服务器返回的数据并打印到终端，实现被动接收
- 安全退出机制：输入 exit 命令即可主动关闭连接，安全退出事件循环
 
感谢大家的下载、收藏与贡献。
 
 
 
Embedded TCP Bidirectional Chat Client
 
Basic Introduction
 
This software is an embedded C language project based on the OpenWrt platform. It uses the  usock / uloop  libraries from libubox to implement TCP communication and event-driven architecture, combined with the UCI configuration system. It builds a client program step-by-step that enables bidirectional chat with PC-side network debugging tools.
 
Software Features
 
Adopting a modular design for embedded development scenarios, the software has the following characteristics:
 
- Modular Architecture: The code is split into three independent modules: configuration reading, network communication, and keyboard input, with a clear structure for easy maintenance and expansion
- UCI Configuration Management: Reads the server IP and port from the UCI configuration file, allowing quick switching of target servers without modifying code
- Event-Driven IO: Based on the uloop event loop, it handles both network reception and keyboard input simultaneously, fully non-blocking without relying on multi-threading
- Minimal Implementation: Only depends on  libubox  and  uci  libraries, with no redundant dependencies, suitable for resource-constrained embedded devices
- Lightweight Chat: Supports bidirectional text chat with PC-side network debugging tools, and allows safe exit by entering  exit 
 
Main Functions
 
The core functions of the software meet the needs of network debugging and bidirectional communication between embedded devices and PCs:
 
- Automatic Configuration Loading: Automatically reads the server IP and port from the UCI configuration file on startup to complete initialization
- TCP Client Connection: Implements a stable TCP client based on usock, which can establish a connection with a PC-side TCP server
- Keyboard Input Sending: Supports sending text data to the server via standard input for active communication
- Server Data Reception: Real-time listens for data returned by the server and prints it to the terminal for passive reception
- Safe Exit Mechanism: Entering the  exit  command will actively close the connection and safely exit the event loop
 
Thank you for your downloads, stars, and contributions.
 
 
 
补充：编译与运行说明
 
项目结构
 
plaintext
  
/root/test/com/
├── CMakeLists.txt   # 编译脚本
├── main.c           # 主程序入口
├── usock/           # 网络通信模块
├── stdin/           # 键盘输入模块
└── read/            # UCI配置读取模块
 
 
1. 配置文件准备
 
按路径  /root/test/com/client  创建UCI配置文件，修改为你的电脑服务器IP/端口：
 
bash
  
cat > /root/test/com/client << EOF
config client 'demo'
    option ip '192.168.4.94'
    option port '61000'
EOF
 
 
2. 编译命令
 
bash
  
cd /root/test/com
rm -rf build && mkdir build && cd build
cmake ..
make
 
 
3. 运行前准备
 
在电脑端打开TCP调试工具，创建与配置文件IP/端口一致的TCP服务器。
 
4. 运行程序
 
bash
  
./com
