#include "Server.h"
#include "MessageDef.h"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h> // File control
#include <cerrno>
#include <iostream>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

static constexpr int MAX_EVENTS = 1024;
static constexpr int READ_BUFFER = 4096;

Server::Server(uint16_t _port): serverFd(-1), epollFd(-1), port(_port), running(false) {}

/*
 * socket -> bind -> listen -> create epoll
 */
void Server::start() {
    // socket
    if(this->serverFd != -1) {
        this->serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    }
    if (serverFd < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    int flags = fcntl(serverFd, F_GETFL, 0);
    fcntl(serverFd, F_SETFL, flags | O_NONBLOCK);

    // bind
    sockaddr_in addr{};
    addr.sin_family      = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port        = htons(this->port);

    if (bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    // listen
    if (listen(serverFd, SOMAXCONN) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
    std::cout << "ChatServer listening on port " << port << std::endl;

    // create epoll
    this->epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    epoll_event ev{};
    ev.events   = EPOLLIN; // EPOLLIN: readable event, EPOLLOUT: writable event
    ev.data.fd  = this->serverFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1) { // Add server itself
        perror("epoll_ctl: serverFd");
        exit(EXIT_FAILURE);
    }
}

void Server::acceptNewClient() {
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t   addrLen = sizeof(clientAddress);

        int clientFd = accept(serverFd, (sockaddr*)&clientAddress, &addrLen);
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                return;
            } else {
                perror("accept");
                return;
            }
        }

        int flags = fcntl(clientFd, F_GETFL, 0);
        fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

        epoll_event ev{};
        ev.events  = EPOLLIN | EPOLLET;
        ev.data.fd = clientFd;
        if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0) {
            perror("epoll_ctl: clientFd");
            close(clientFd);
        }

        int tempUserId = clientFd;
        UserSession session(clientFd, tempUserId);
        session.setLoggedIn(false);
        session.updateLastActive();
        userSessions[tempUserId] = session;

        std::cout << "New client connected: fd=" << clientFd << std::endl;
    }
}

void Server::run() {
    this->start();

    std::vector<epoll_event> events(MAX_EVENTS);
    while (true) {
        int n = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; i++) {
            auto &ev = events[i];
            if (ev.data.fd == this -> serverFd) {
                this->acceptNewClient();
            } else if (ev.events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
                // EPOLLERR: event error, EPOLLHUP: event hang up
                this->handleClientMessage(ev.data.fd);
            }
        }
    }
}

void Server::handleClientMessage(int clientFd) {
    char buffer[READ_BUFFER];
    while (true) {
        ssize_t bytesRead = ::recv(clientFd, buffer, sizeof(buffer), 0);
        if (bytesRead < 0) {
            if (errno == EAGAIN or errno == EWOULDBLOCK) break;
            perror("recv");
            closeClient(clientFd);
            return;
        } else if (bytesRead == 0) {
            closeClient(clientFd);
            return;
        }

        std::string raw(buffer, bytesRead);
        Message msg = parseMessage(raw);

        switch (msg.type) {
            case MessageType::Private:
                this->routePrivateMessage(msg);
                break;
            case MessageType::Group:
                this->routeGroupMessage(msg);
                break;
            case MessageType::System:
                this->handleSystemMessage(msg);
                break;
        }

        persistMessage(msg);
    }
}
