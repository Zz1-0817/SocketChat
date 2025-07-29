#pragma once

#include <cstdint>
#include <vector>
#include <string>

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

