#include "DataAccessObject/User.h"

bool UserDAO::insertUser(const User& user) {
    return SqlHelper::execute(
        db_, "INSERT INTO users (username, nickname) VALUES (?, ?);",
        {user.username, user.nickname});
}

std::optional<User> UserDAO::getUserByUsername(const std::string& username) {
    const std::string sql =
        "SELECT id, username, nickname FROM users WHERE username = ?;";
    auto rows = SqlHelper::query(db_, sql, {username});
    if (rows.empty()) {
        return std::nullopt;
    }

    const auto& row = rows[0];
    if (row.size() < 3) {
        return std::nullopt;
    }

    User user;
    user.id = std::stoi(row[0]);
    user.username = row[1];
    user.nickname = row[2];
    return user;

    if (!rows.empty()) {
        return User{std::stoi(rows[0][0]), rows[0][1], rows[0][2]};
    }
    return std::nullopt;
}

bool UserDAO::verifyPassword(const std::string& username, const std::string& password) {
    const std::string sql = "SELECT password FROM users WHERE username = ?";
    auto rows = SqlHelper::query(db_, sql, {username});

    if (rows.empty() or rows[0].empty()) return false;

    const std::string& storedPassword = rows[0][0];
    return storedPassword == password;
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
