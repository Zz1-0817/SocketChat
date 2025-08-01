#pragma once

#include <cstdint>
#include <unordered_map>
#include <string>
#include "MessageDef.h"
#include "UserSession.h"
#include "ChatRoom.h"

class Server {
public:
    Server(uint16_t _port);
    ~Server();

    void start();
    void stop();

    void run();

    void acceptNewClient();
    void closeClient(int clientFd);

    void handleClientMessage(int clientFd);

private:
    int  serverFd;
    int  epollFd;
    uint16_t port;
    bool running;

    std::unordered_map<int, UserSession> userSessions;
    std::unordered_map<int, ChatRoom> chatRooms;

    void routePrivateMessage(const Message& msg);
    void routeGroupMessage(const Message& msg);
    void handleSystemMessage(const Message& msg);

    void persistMessage(const Message& msg);

    std::string serialize(const Message& msg);
    Message parseMessage(std::string raw);
};
