#include "ChatServer.h"
#include <unistd.h>
#include <fcntl.h> // File control
#include <cerrno>
#include <iostream>
#include <algorithm>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <arpa/inet.h>

static constexpr int MAX_EVENTS = 1024;
static constexpr int READ_BUFFER = 4096;

ChatServer::ChatServer(uint16_t port_)
  : serverFd(-1), epollFd(-1), port(port_)
{}

ChatServer::~ChatServer() {
    if (serverFd >= 0) close(serverFd);
    if (epollFd  >= 0) close(epollFd);
    for (int fd : clients) close(fd);
}

void ChatServer::initServerSocket() {
    // socket
    serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
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
    addr.sin_port        = htons(port);

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
}

// initial and run the main loop
void ChatServer::run() {
    initServerSocket();

    // create an epoll instance and add the socket
    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    epoll_event ev{};
    ev.events   = EPOLLIN; // EPOLLIN: readable event, EPOLLOUT: writable event
    ev.data.fd  = this->serverFd;
    /*
     * RETURN VALUE
     * When successful,  epoll_ctl()  returns  zero.   When  an  error  occurs,
     * epoll_ctl() returns -1 and errno is set to indicate the error.
     */
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev) == -1) { // Add server itself
        perror("epoll_ctl: serverFd");
        exit(EXIT_FAILURE);
    }

    std::vector<epoll_event> events(MAX_EVENTS);
    while (true) {
        int n = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR) continue;
            perror("epoll_wait");
            break;
        }
        for (int i = 0; i < n; ++i) {
            auto &e = events[i];
            if (e.data.fd == this->serverFd) {
                handleNewConnection();
            } else if (e.events & (EPOLLIN | EPOLLERR | EPOLLHUP)) {
                // EPOLLERR: event error, EPOLLHUP: event hang up
                handleClientMessage(e.data.fd);
            }
        }
    }
}

void ChatServer::handleNewConnection() {
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t   len = sizeof(clientAddress);
        // accept a connection on a socket
        int clientFd = accept(serverFd, (sockaddr*)&clientAddress, &len);
        if (clientFd < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("accept");
            break;
        }

        int flags = fcntl(clientFd, F_GETFL, 0);
        fcntl(clientFd, F_SETFL, flags | O_NONBLOCK);

        epoll_event ev{};
        ev.events  = EPOLLIN | EPOLLET;
        ev.data.fd = clientFd;
        if (epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0) {
            perror("epoll_ctl: clientFd");
            close(clientFd);
            continue;
        }

        clients.push_back(clientFd);
        std::cout << "New client connected: fd=" << clientFd << std::endl;
    }
}

void ChatServer::handleClientMessage(int clientFd) {
    char buf[READ_BUFFER];
    while (true) {
        ssize_t cnt = ::read(clientFd, buf, sizeof(buf));
        /*
         * RETURN VALUE
         * On  success, the number of bytes read is returned (zero indicates end of
         * file), and the file position is advanced by this number.  It is  not  an
         * error if this number is smaller than the number of bytes requested; this
         * may  happen for example because fewer bytes are actually available right
         * now (maybe because we were close to end-of-file, or because we are read‐
         * ing from a pipe, or from a terminal), or because read() was  interrupted
         * by a signal.  See also NOTES.

         * On  error,  -1  is returned, and errno is set to indicate the error.  In
        * this case, it is left unspecified whether the  file  position  (if  any) changes.
        */
        if (cnt < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                break;
            }
            perror("read");
            cnt = 0;
        }
        if (cnt == 0) {
            std::cout << "Client disconnected: fd=" << clientFd << std::endl;
            close(clientFd);
            epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
            this->clients.erase(std::remove(this->clients.begin(), this->clients.end(), clientFd),
                          this->clients.end());
            return;
        }
        std::string msg(buf, cnt);
        broadcastMessage(msg, clientFd);
    }
}

void ChatServer::broadcastMessage(const std::string &msg, int senderFd) {
    for (int fd : clients) {
        if (fd == senderFd) continue;
        ssize_t sent = ::send(fd, msg.data(), msg.size(), 0);
        if (sent < 0 && sent != (ssize_t)msg.size()) {
        }
    }
}

