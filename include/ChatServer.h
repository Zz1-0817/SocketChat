#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "Socket.h"

class ChatServer : public Socket {
private:
    int         epollFd;
    std::vector<int> clients;

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

