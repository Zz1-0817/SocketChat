#include <DataAccessObject/Message.h>

bool MessageDAO::insertMessage(const Message& msg) {
    return SqlHelper::execute(db_,
        "INSERT INTO messages (room_id, sender_id, content, timestamp) VALUES (?, ?, ?, ?);",
        {std::to_string(msg.target), std::to_string(msg.senderId), msg.content, msg.timestamp});
}

std::vector<Message> MessageDAO::getMessagesByRoom(int roomId, int limit) {
    auto rows = SqlHelper::query(db_,
        "SELECT id, room_id, sender_id, content, timestamp FROM messages WHERE room_id = ? ORDER BY id DESC LIMIT ?;",
        {std::to_string(roomId), std::to_string(limit)});

    std::vector<Message> result;
    for (const auto& row : rows) {
        result.push_back(Message{
            std::stoi(row[0]), std::stoi(row[1]), std::stoi(row[2]), row[3], row[4]
        });
    }
    return result;
}

std::vector<Message> MessageDAO::getMessagesByUser(int userId) {
    auto rows = SqlHelper::query(db_,
        "SELECT id, room_id, sender_id, content, timestamp FROM messages WHERE sender_id = ?;",
        {std::to_string(userId)});

    std::vector<Message> result;
    for (const auto& row : rows) {
        result.push_back(Message{
            std::stoi(row[0]), std::stoi(row[1]), std::stoi(row[2]), row[3], row[4]
        });
    }
    return result;
}

