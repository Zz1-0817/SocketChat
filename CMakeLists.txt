cmake_minimum_required(VERSION 3.10)
project(ChatServer)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 包含头文件目录
include_directories(include)

# 收集源文件
set(SOURCES
    src/ChatServer.cpp
    test/Chatserver.cpp
)

# 生成可执行文件
add_executable(chat-server ${SOURCES})

