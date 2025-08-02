#pragma once

#include <ChatRoom.h>
#include <Session.h>
#include <TypeDef.h>
#include <DataAccessObject/User.h>
#include <DataAccessObject/ChatRoom.h>

#include <cstdint>
#include <string>
#include <unordered_map>

class Server {
   public:
    Server(uint16_t _port, UserDAO &_userDAO, ChatRoomDAO &_chatRoomDAO);
    ~Server();

    void start();
    void stop();

    void run();

    void acceptNewClient();
    void closeClient(int clientFd);

    void handleClientMessage(int clientFd);

   private:
    int serverFd;
    int epollFd;
    uint16_t port;
    bool running;
    UserDAO &userDAO;
    ChatRoomDAO &chatRoomDAO;
    std::unordered_map<int, Session> userSessions; // key: clientFd, value: Session
    std::unordered_map<int, ChatRoom> chatRooms; // key: roomId, value: ChatRoom

    void routePrivateMessage(const Message& msg);
    void routeGroupMessage(const Message& msg);
    void handleSystemMessage(const Message& msg);

    void persistMessage(const Message& msg);

    std::string serialize(const Message& msg);
    Message parseMessage(std::string raw);
};
