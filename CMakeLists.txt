cmake_minimum_required(VERSION 3.14)
project(ChatServer)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# 查找 SQLite3 库
find_package(PkgConfig REQUIRED)
pkg_check_modules(SQLITE3 REQUIRED sqlite3)

# 包含头文件路径
include_directories(
    include
    external
    ${SQLITE3_INCLUDE_DIRS}
)

# 设置源文件（主程序 + 所有模块）
file(GLOB_RECURSE SRC_FILES
    src/*.cpp
    src/*/*.cpp
)

# 添加 main.cpp
set(MAIN_FILE main.cpp)

# 构建可执行文件
add_executable(ChatServer ${MAIN_FILE} ${SRC_FILES})

# 链接 SQLite3 库
target_link_libraries(ChatServer PRIVATE ${SQLITE3_LIBRARIES})

# 如果你有 test 模块
# enable_testing()
# add_subdirectory(test)

