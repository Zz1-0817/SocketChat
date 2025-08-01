#include "DataAccessObject/ChatRoom.h"

bool ChatRoomDAO::createRoom(const std::string& name) {
    return SqlHelper::execute(db_, "INSERT INTO chat_rooms (name) VALUES (?);",
                              {name});
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
