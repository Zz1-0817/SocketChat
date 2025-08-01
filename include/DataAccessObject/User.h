#pragma once
#include <DataAccessObject/Helper.h>
#include <TypeDef.h>

#include <optional>

class UserDAO {
   public:
    explicit UserDAO(sqlite3* db) : db_(db) {}

    bool insertUser(const User& user);
    std::optional<User> getUserByUsername(const std::string& username);
    bool updateNickname(int userId, const std::string& newNickname);
    bool deleteUser(int userId);
    bool updateStatus(int userId, int status);
    bool verifyPassword(const std::string& username,
                        const std::string& password);

   private:
    sqlite3* db_;
};
