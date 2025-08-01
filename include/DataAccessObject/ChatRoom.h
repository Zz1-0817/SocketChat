#pragma once
#include <ChatRoom.h>
#include <DataAccessObject/Helper.h>

#include <optional>
#include <vector>

class ChatRoomDAO {
   public:
    explicit ChatRoomDAO(sqlite3* db) : db_(db) {}

    bool createRoom(const std::string& name);
    std::optional<ChatRoom> getRoomById(int roomId);
    std::vector<ChatRoom> listAllRooms();
    bool deleteRoom(int roomId);

   private:
    sqlite3* db_;
};
