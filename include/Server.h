#pragma once

#include <cstdint>
#include <unordered_map>
#include <vector>
#include <string>
#include "MessageDef.h"
#include "UserSession.h"
#include "ChatRoom.h"

class ChatServer {
private:
    int         serverFd;
    int         epollFd;
    std::vector<int> clients;
    uint16_t    port;

    void initServerSocket();
    void handleEvents();
    void handleNewConnection();
    void handleClientMessage(int clientFd);
    void broadcastMessage(const std::string &msg, int senderFd);

public:
    explicit ChatServer(uint16_t port);
    ~ChatServer();

    void run();
};

class MessageServer {
public:
    MessageServer();
    ~MessageServer();

    bool start(int port);
    void stop();

    void run();

    void acceptNewClient();
    void closeClient(int clientFd);

    void handleClientMessage(int clientFd);

private:
    int listenFd;
    int epollFd;
    bool running;

    std::unordered_map<int, UserSession> userSessions;
    std::unordered_map<int, ChatRoom> chatRooms;

    void routePrivateMessage(const Message& msg);
    void routeGroupMessage(const Message& msg);
    void handleSystemMessage(const Message& msg);

    void persistMessage(const Message& msg);

    std::string serialize(const Message& msg);
    Message parseMessage(int clientFd);
};
