#pragma once
#include <ChatRoom.h>
#include <DataAccessObject/Helper.h>

#include <optional>
#include <vector>

class ChatRoomDAO {
   public:
    explicit ChatRoomDAO(sqlite3* db) : db_(db) {}

    bool createRoom(const std::string& name);
    bool deleteRoom(int roomId);

    std::optional<ChatRoom> getRoomById(int roomId);
    std::vector<ChatRoom> getRoomsByUser(int userId);
    std::unordered_set<int> getJoinedRoomIdsByUser(int userId);
    std::vector<ChatRoom> listAllRooms();

   private:
    sqlite3* db_;
};
