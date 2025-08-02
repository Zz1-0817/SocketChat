// src/main.cpp

#include <sqlite3.h>

#include "Server.h"
#include "DataAccessObject/User.h"
#include "DataAccessObject/ChatRoom.h"
#include "DataAccessObject/Message.h"

#include <iostream>
#include <string>
#include <cstdint>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "用法: " << argv[0]
                  << " <数据库文件路径> <端口号>\n";
        return 1;
    }

    const std::string dbPath = argv[1];
    int portInt = 0;
    try {
        portInt = std::stoi(argv[2]);
        if (portInt < 1 || portInt > 65535) throw 0;
    } catch (...) {
        std::cerr << "端口号必须是 1–65535 之间的整数\n";
        return 1;
    }
    uint16_t port = static_cast<uint16_t>(portInt);

    // 打开 SQLite3 数据库
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "无法打开数据库 '" << dbPath
                  << "': " << sqlite3_errmsg(db) << "\n";
        return 1;
    }

    // 构造各 DAO（以 sqlite3* 为参数）
    UserDAO     userDAO(db);
    ChatRoomDAO chatRoomDAO(db);
    MessageDAO  messageDAO(db);

    // 构造并启动 Server
    Server server(port, userDAO, chatRoomDAO, messageDAO);
    server.start();
    std::cout << "服务器已启动，监听端口 " << port << "\n";
    server.run();

    // 退出前关闭数据库
    sqlite3_close(db);
    return 0;
}

