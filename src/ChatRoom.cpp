#include "ChatRoom.h"

ChatRoom::ChatRoom(int roomId, const std::string& name)
    : roomId(roomId), name(name) {}

int ChatRoom::getRoomId() const { return roomId; }

std::string ChatRoom::getName() const { return name; }

void ChatRoom::setName(const std::string& newName) { name = newName; }

bool ChatRoom::addMember(int userId) {
    return memberIds.insert(userId).second;  // true if inserted
}

bool ChatRoom::removeMember(int userId) {
    return memberIds.erase(userId) > 0;  // true if erased
}

bool ChatRoom::isMember(int userId) const {
    return memberIds.count(userId) > 0;
}

const std::unordered_set<int>& ChatRoom::getMembers() const {
    return memberIds;
}

void ChatRoom::addMessage(const Message& msg) { messageHistory.push_back(msg); }

const std::vector<Message>& ChatRoom::getRecentMessages() const {
    return messageHistory;
}
