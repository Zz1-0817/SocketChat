#include <Server.h>
#include <string>
#include <iostream>

int main () {
    int port = 8080;
    std::string dbPath = "./test/test.db";
    sqlite3* db = nullptr;
    if (sqlite3_open(dbPath.c_str(), &db) != SQLITE_OK) {
        std::cerr << "无法打开数据库 '" << dbPath
                  << "': " << sqlite3_errmsg(db) << "\n";
        return 1;
    }
    UserDAO     userDAO(db);
    ChatRoomDAO chatRoomDAO(db);
    MessageDAO  messageDAO(db);
    Server Server(port, userDAO, chatRoomDAO, messageDAO);
    Server.run();

    return 0;
}
