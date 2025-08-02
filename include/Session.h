#pragma once
#include <DataAccessObject/ChatRoom.h>
#include <DataAccessObject/User.h>

#include <string>
#include <unordered_set>

class Session {
   public:
    bool websocket = false;
    Session(int socketFd, UserDAO &userDAO, ChatRoomDAO &chatRoomDAO)
        : userDAO(userDAO), chatRoomDAO(chatRoomDAO) {};

    bool login(const std::string &username, const std::string &password);
    void logout();
    bool switchUser(const std::string &newUsername,
                    const std::string &newPassword);

    int getUserId() const;
    int getSocketFd() const;
    std::string getUsername() const;
    std::string getNickname() const;
    const std::unordered_set<int> &getJoinedRoomIds() const;

    bool isLoggedIn() const;
    bool isDisconnected() const;
    bool isInRoom(int roomId) const;

    void joinRoom(int roomId);
    void setLoggedIn(bool status);
    void setNickname(const std::string &name);
    void markDisconnected();

   private:
    int socketFd;
    int userId;
    std::string username;
    std::string nickname;
    bool loggedIn;
    bool disconnected;
    std::unordered_set<int> joinedRoomIds;

    UserDAO &userDAO;
    ChatRoomDAO &chatRoomDAO;
};
