#pragma once
#include <sqlite3.h>
#include <string>
#include <vector>

class SqlHelper {
public:
    static bool execute(sqlite3* db, const std::string& sql, const std::vector<std::string>& params);

    static std::vector<std::vector<std::string>> query(sqlite3* db, const std::string& sql, const std::vector<std::string>& params);
};
