#include <DataAccessObject/Message.h>

bool MessageDAO::insertMessage(const Message& msg) {
    std::string sql = R"(
        INSERT INTO messages (sender, receiver, room_id, timestamp, content, type)
        VALUES (?, ?, ?, ?, ?, ?);
    )";

    std::string receiverStr = (msg.type == MessageType::Private)
                                  ? std::to_string(msg.to)
                                  : "";

    std::string roomStr = (msg.type == MessageType::Group)
                              ? std::to_string(msg.to)
                              : "";

    return SqlHelper::execute(
        db_, sql,
        {std::to_string(msg.from), receiverStr, roomStr,
         std::to_string(msg.timestamp), msg.content,
         std::to_string(static_cast<int>(msg.type))});
}

std::vector<Message> MessageDAO::getRecentMessagesByRoom(int roomId,
                                                         int limit) {
    std::string sql = R"(
        SELECT sender, content, timestamp, type
        FROM messages
        WHERE room_id = ?
        ORDER BY timestamp DESC
        LIMIT ?
    )";

    auto rows = SqlHelper::query(
        db_, sql, {std::to_string(roomId), std::to_string(limit)});

    std::vector<Message> messages;
    for (const auto& row : rows) {
        Message msg;
        msg.from = std::stoi(row[0]);
        msg.content = row[1];
        msg.timestamp = std::stoull(row[2]);
        msg.type = static_cast<MessageType>(std::stoi(row[3]));
        msg.to = roomId;
        messages.push_back(std::move(msg));
    }
    return messages;
}

std::vector<Message> MessageDAO::getRecentMessagesByUsers(int userA, int userB, int limit) {
    std::string sql = R"(
        SELECT sender, receiver, content, timestamp, type
        FROM messages
        WHERE type = ?
          AND (
              (sender = ? AND receiver = ?)
              OR
              (sender = ? AND receiver = ?)
          )
        ORDER BY timestamp DESC
        LIMIT ?
    )";

    auto rows = SqlHelper::query(
        db_, sql,
        {std::to_string(static_cast<int>(MessageType::Private)),
         std::to_string(userA), std::to_string(userB), std::to_string(userB),
         std::to_string(userA), std::to_string(limit)});

    std::vector<Message> messages;
    for (const auto& row : rows) {
        Message msg;
        msg.from = std::stoi(row[0]);
        msg.to = std::stoi(row[1]);
        msg.content = row[2];
        msg.timestamp = std::stoull(row[3]);
        msg.type = static_cast<MessageType>(std::stoi(row[4]));
        messages.push_back(std::move(msg));
    }
    return messages;
}
