#include "DataAccessObject/Helper.h"

#include <iostream>

using std::vector, std::string;

bool SqlHelper::execute(sqlite3 *db, const string &sql,
                        const vector<string> &params) {
    sqlite3_stmt *stmt = nullptr;
    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return false;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1,
                          SQLITE_STATIC);
    }

    bool success = (sqlite3_step(stmt) == SQLITE_DONE);
    sqlite3_finalize(stmt);
    return success;
}

vector<vector<string>> SqlHelper::query(sqlite3 *db, const string &sql,
                                        const vector<string> &params) {
    sqlite3_stmt *stmt = nullptr;
    vector<vector<string>> results;

    if (sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr) != SQLITE_OK) {
        std::cerr << "Prepare failed: " << sqlite3_errmsg(db) << "\n";
        return results;
    }

    for (size_t i = 0; i < params.size(); ++i) {
        sqlite3_bind_text(stmt, static_cast<int>(i + 1), params[i].c_str(), -1,
                          SQLITE_STATIC);
    }

    while (sqlite3_step(stmt) == SQLITE_ROW) {
        int colCount = sqlite3_column_count(stmt);
        vector<string> row;
        for (int i = 0; i < colCount; ++i) {
            const unsigned char *text = sqlite3_column_text(stmt, i);
            row.emplace_back(text ? reinterpret_cast<const char *>(text) : "");
        }
        results.push_back(std::move(row));
    }

    sqlite3_finalize(stmt);
    return results;
}
