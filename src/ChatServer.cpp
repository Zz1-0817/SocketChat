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

ChatServer::ChatServer(uint16_t port_) : Socket(), epollFd(-1)
{
    port = port_;
}

ChatServer::~ChatServer() {
    close(fd);
    close(epollFd);
    for (int FD : clients) close(FD);
}

// initial and run the main loop
void ChatServer::run() {
    Socket::set_non_blocking();
    Socket::bind("127.0.0.1", port);
    Socket::listen(SOMAXCONN);

    // create an epoll instance and add the socket
    epollFd = epoll_create1(0);
    if (epollFd < 0) {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }

    epoll_event ev{};
    ev.events   = EPOLLIN; // EPOLLIN: readable event, EPOLLOUT: writable event
    ev.data.fd  = this-> fd;
    /*
     * RETURN VALUE
     * When successful,  epoll_ctl()  returns  zero.   When  an  error  occurs,
     * epoll_ctl() returns -1 and errno is set to indicate the error.
     */
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, fd, &ev) == -1) { // Add server itself
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
            if (e.data.fd == this -> fd) {
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
        int clientFd = Socket::accept();

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
            std::remove(this->clients.begin(), this->clients.end(), clientFd);
            return;
        }
        std::string msg(buf, cnt);
        broadcastMessage(msg, clientFd);
    }
}

void ChatServer::broadcastMessage(const std::string &msg, int senderFd) {
    for (int FD : clients) {
        if (FD == senderFd) continue;
        ssize_t sent = Socket::send(FD, msg.data(), msg.size());
    }
}

