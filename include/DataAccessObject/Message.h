#pragma once
#include <DataAccessObject/Helper.h>
#include <TypeDef.h>

#include <vector>

class MessageDAO {
   public:
    explicit MessageDAO(sqlite3* db) : db_(db) {}

    bool insertMessage(const Message& msg);
    std::vector<Message> getMessagesByRoom(int roomId, int limit = 50);
    std::vector<Message> getMessagesByUser(int userId);

   private:
    sqlite3* db_;
};
