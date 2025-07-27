#pragma once
#include <cstdint>
#include <vector>
#include <string>

class ChatServer {
private:
    int serverFd;
    std::vector<int> clients;
    uint16_t port;

    void initServerSocket();
    void handleConnections();
    void handleClientMessage(int clientFd);
    void broadcastMessage(const std::string &msg, int senderFd);
public:
    ChatServer(int port);
    void run();
};
