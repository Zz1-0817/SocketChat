#include "ChatServer.h"
#include <asm-generic/socket.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <stdexcept>
#include <cstring>

ChatServer::ChatServer(int _port) : port(_port) {}

void ChatServer::initServerSocket() {
    // Socket Create
    this->serverFd = socket(AF_INET, SOCK_STREAM, 0);

    if(this->serverFd < 0) {
        throw std::runtime_error("Failed to create socket!");
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(this->port);

    // Socket Bind

    if(bind(this->serverFd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        close(this->serverFd);
        throw std::runtime_error("Bind Failed");
    }

    // Socket Listen
    if(listen(this->serverFd, SOMAXCONN) < 0) {
        close(this->serverFd);
        throw std::runtime_error("Listen failed");
    }
}
