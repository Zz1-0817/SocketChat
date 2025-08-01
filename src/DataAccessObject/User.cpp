#include "DataAccessObject/User.h"

bool UserDAO::insertUser(const User& user) {
    return SqlHelper::execute(
        db_, "INSERT INTO users (username, nickname) VALUES (?, ?);",
        {user.username, user.nickname});
}

std::optional<User> UserDAO::getUserByUsername(const std::string& username) {
    auto rows = SqlHelper::query(
        db_, "SELECT id, username, nickname FROM users WHERE username = ?;",
        {username});
    if (!rows.empty()) {
        return User{std::stoi(rows[0][0]), rows[0][1], rows[0][2]};
    }
    return std::nullopt;
}

bool UserDAO::updateNickname(int userId, const std::string& newNickname) {
    return SqlHelper::execute(db_,
                              "UPDATE users SET nickname = ? WHERE id = ?;",
                              {newNickname, std::to_string(userId)});
}

bool UserDAO::deleteUser(int userId) {
    return SqlHelper::execute(db_, "DELETE FROM users WHERE id = ?;",
                              {std::to_string(userId)});
}
