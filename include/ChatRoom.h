#pragma once
#include <TypeDef.h>

#include <string>
#include <unordered_set>
#include <vector>

class ChatRoom {
   public:
    ChatRoom(int roomId, const std::string& name);

    int getRoomId() const;
    std::string getName() const;
    void setName(const std::string& newName);

    bool addMember(int userId);
    bool removeMember(int userId);
    bool isMember(int userId) const;
    const std::unordered_set<int>& getMembers() const;

    void addMessage(const Message& msg);
    const std::vector<Message>& getRecentMessages() const;

   private:
    int roomId;
    std::string name;

    std::unordered_set<int> memberIds;
    std::vector<Message> messageHistory;
};
