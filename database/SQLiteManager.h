#pragma once
#include <sqlite3.h>
#include <string>

class SQLiteManager {
private:
    sqlite3* db;

public:
    explicit SQLiteManager(const std::string& dbFile);
    ~SQLiteManager();

    bool execute(const std::string& sql);
    bool query(const std::string& sql, int (*callback)(void*, int, char**, char**), void* data);
};
