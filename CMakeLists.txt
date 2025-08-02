# 项目根目录下的 CMakeLists.txt

cmake_minimum_required(VERSION 3.10)
project(ChatServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)    # 用于 SHA1/Base64
find_package(SQLite3 REQUIRED)    # 如果找不到，可改为直接 -lsqlite3

# 包含目录
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${PROJECT_SOURCE_DIR}/src
  ${PROJECT_SOURCE_DIR}/src/DataAccessObject
)

# 源文件列表
file(GLOB_RECURSE DAO_SRCS
  "${PROJECT_SOURCE_DIR}/src/DataAccessObject/*.cpp"
)
set(SERVER_SRCS
  src/main.cpp
  src/Server.cpp
  src/Session.cpp
  src/ChatRoom.cpp
  ${DAO_SRCS}
)

# 生成可执行文件
add_executable(server ${SERVER_SRCS})

# 链接库
target_link_libraries(server PRIVATE
  Threads::Threads
  OpenSSL::SSL
  OpenSSL::Crypto
  ${SQLite3_LIBRARIES}
)

# 如果 find_package(SQLite3) 失败，改为下面一行：
# target_link_libraries(server PRIVATE sqlite3)

