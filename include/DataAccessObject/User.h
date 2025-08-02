#pragma once
#include <DataAccessObject/Helper.h>
#include <TypeDef.h>

#include <optional>

class UserDAO {
   public:
    explicit UserDAO(sqlite3* db) : db_(db) {}

    std::optional<User> getUserByUsername(const std::string& username);
    bool updateStatus(int userId, int status);
    bool verifyPassword(const std::string& username,
                        const std::string& password);

    bool insertUser(const User& user);
    bool updateNickname(int userId, const std::string& newNickname);
    bool deleteUser(int userId);

   private:
    sqlite3* db_;
};
