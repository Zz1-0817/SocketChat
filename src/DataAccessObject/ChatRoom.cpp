#include "DataAccessObject/ChatRoom.h"

bool ChatRoomDAO::createRoom(const std::string& name) {
    const std::string sql = "INSERT INTO chat_rooms (name) VALUES (?);";
    return SqlHelper::execute(db_, sql, {name});
}

std::optional<ChatRoom> ChatRoomDAO::getRoomById(int roomId) {
    auto rows =
        SqlHelper::query(db_, "SELECT id, name FROM chat_rooms WHERE id = ?;",
                         {std::to_string(roomId)});
    if (!rows.empty()) {
        return ChatRoom{std::stoi(rows[0][0]), rows[0][1]};
    }
    return std::nullopt;
}

std::vector<ChatRoom> ChatRoomDAO::listAllRooms() {
    auto rows = SqlHelper::query(db_, "SELECT id, name FROM chat_rooms;", {});
    std::vector<ChatRoom> result;
    for (const auto& row : rows) {
        result.push_back(ChatRoom{std::stoi(row[0]), row[1]});
    }
    return result;
}

bool ChatRoomDAO::deleteRoom(int roomId) {
    return SqlHelper::execute(db_, "DELETE FROM chat_rooms WHERE id = ?;",
                              {std::to_string(roomId)});
}

std::unordered_set<int> ChatRoomDAO::getJoinedRoomIdsByUser(int userId) {
    std::string sql = "SELECT room_id FROM room_members WHERE user_id = ?";
    auto rows = SqlHelper::query(db_, sql, { std::to_string(userId) });

    std::unordered_set<int> roomIds;
    for (const auto& row : rows) {
        roomIds.insert(std::stoi(row[0]));
    }
    return roomIds;
}

