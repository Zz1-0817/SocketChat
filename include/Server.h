#pragma once

#include <ChatRoom.h>
#include <DataAccessObject/ChatRoom.h>
#include <DataAccessObject/User.h>
#include <Session.h>
#include <TypeDef.h>

#include <cstdint>
#include <unordered_map>

#include "DataAccessObject/Message.h"

class Server {
   public:
    Server(uint16_t _port, UserDAO& _userDAO, ChatRoomDAO& _chatRoomDAO,
           MessageDAO& _messageDAO);
    ~Server() = default;

    void start();
    void stop();

    void run();

    void acceptNewClient();
    void closeClient(int clientFd);

    void handleClientMessage(int clientFd);
    bool verifyToken(int clientFd, const std::string& nxtToken);

   private:
    int serverFd;
    int epollFd;
    uint16_t port;
    bool running;
    UserDAO& userDAO;
    ChatRoomDAO& chatRoomDAO;
    MessageDAO& messageDAO;

    std::unordered_map<int, Session>
        userSessions;  // key: clientFd, value: Session
    std::unordered_map<int, ChatRoom>
        chatRooms;                            // key: roomId, value: ChatRoom
    std::unordered_map<int, int> userIdToFd;  // key: userId, value: Fd

    void routePrivateMessage(const Message& msg);
    void routeGroupMessage(const Message& msg);
    void handleSystemMessage(int clientFd, const Request& request, const Message& msg);

    void persistMessage(const Message& msg);
};
