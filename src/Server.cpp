#include <Server.h>
#include <TypeDef.h>
#include <Utils.h>
#include <arpa/inet.h>
#include <fcntl.h>  // File control
#include <sys/epoll.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>

#include <cerrno>
#include <iostream>
#include <json.hpp>

using json = nlohmann::json;

static constexpr int MAX_EVENTS = 1024;
static constexpr int READ_BUFFER = 8192;

Server::Server(uint16_t _port, UserDAO& _userDAO, ChatRoomDAO& _chatRoomDAO,
               MessageDAO& _messageDAO)
    : port(_port),
      userDAO(_userDAO),
      chatRoomDAO(_chatRoomDAO),
      messageDAO(_messageDAO) {
    auto rooms = chatRoomDAO.listAllRooms();

    for (auto& room : rooms) {
        chatRooms.insert({room.getRoomId(), room});
    }
}

/*
 * socket -> bind -> listen -> create epoll
 */
void Server::start() {
    // socket
    if (this->serverFd != -1) {
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
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(this->port);

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
    ev.events = EPOLLIN;  // EPOLLIN: readable event, EPOLLOUT: writable event
    ev.data.fd = this->serverFd;
    if (epoll_ctl(epollFd, EPOLL_CTL_ADD, serverFd, &ev) ==
        -1) {  // Add server itself
        perror("epoll_ctl: serverFd");
        exit(EXIT_FAILURE);
    }
}

void Server::acceptNewClient() {
    while (true) {
        sockaddr_in clientAddress{};
        socklen_t addrLen = sizeof(clientAddress);

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
        ev.events = EPOLLIN | EPOLLET;
        ev.data.fd = clientFd;
        if (epoll_ctl(this->epollFd, EPOLL_CTL_ADD, clientFd, &ev) < 0) {
            perror("epoll_ctl: clientFd");
            close(clientFd);
        }

        Session session(clientFd, this->userDAO, this->chatRoomDAO);

        userSessions.insert({clientFd, std::move(session)});
    }
}

void Server::run() {
    this->start();

    std::vector<epoll_event> events(MAX_EVENTS);
    while (true) {
        int n = epoll_wait(epollFd, events.data(), MAX_EVENTS, -1);
        if (n < 0) {
            if (errno == EINTR)
                continue;
            perror("epoll_wait");
            break;
        }

        for (int i = 0; i < n; i++) {
            auto& ev = events[i];
            if (ev.data.fd == this->serverFd) {
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
            if (errno == EAGAIN or errno == EWOULDBLOCK)
                break;
            perror("recv");
            closeClient(clientFd);
            return;
        } else if (bytesRead == 0) {
            closeClient(clientFd);
            return;
        }

        std::string raw(buffer, bytesRead);
        auto request = parseRequest(raw);

        if (request.method == Method::UNKNOWN) {
            return;
        }

        Message msg = parseMessage(request);
        if (msg.type == MessageType::System) {
            this->handleSystemMessage(clientFd, request, msg);
        } else {
            if (this->userIdToFd.find(msg.from) == this->userIdToFd.end()) {
                std::cerr << "User Not online!";
                return;
            }

            if (this->userIdToFd[msg.from] != clientFd) {
                std::cerr << "Wrong id";
                return;
            }

            switch (msg.type) {
                case MessageType::System:
                    break;
                case MessageType::Private:
                    this->routePrivateMessage(msg);
                    break;
                case MessageType::Group:
                    this->routeGroupMessage(msg);
                    break;
            }
        }

        persistMessage(msg);
    }
}

void Server::routePrivateMessage(const Message& msg) {
    auto it = this->userIdToFd.find(msg.to);
    if (it == userIdToFd.end()) {
        std::cerr << "User " << msg.to << " not online\n";
        return;
    }

    int targetFd = it->second;
    std::string response = msgToResponse(msg);
    ::send(targetFd, response.c_str(), response.size(), 0);
}

void Server::routeGroupMessage(const Message& msg) {
    auto it = this->chatRooms.find(msg.to);
    if (it == this->chatRooms.end()) {
        std::cerr << "ChatRoom " << msg.to << " Not Found!\n";
        return;
    }

    std::string response = msgToResponse(msg);
    const auto& memberIds = it->second.getMembers();
    for (int userId : memberIds) {
        if (userId == msg.from)
            continue;
        auto targetIt = this->userIdToFd.find(userId);
        if (targetIt != this->userIdToFd.end()) {
            ::send(targetIt->second, response.c_str(), response.size(), 0);
        }
    }
}

void Server::handleSystemMessage(int clientFd, const Request& request,
                                 const Message& msg) {
    if (request.method != Method::POST) {
        return;
    }

    auto it = this->userSessions.find(this->userIdToFd[msg.from]);
    if (it == userSessions.end())
        return;

    Session& session = it->second;
    json payload = json::parse(msg.content);
    int oldId = session.getUserId();

    if (request.uri == "/login") {
        std::string username = payload.value("username", "");
        std::string password = payload.value("password", "");

        if (session.login(username, password)) {
            userIdToFd[session.getUserId()] = clientFd;

            json response = {{"userId", session.getUserId()},
                             {"username", session.getUsername()},
                             {"nickname", session.getNickname()},
                             {"token", session.getToken()}};
        }

    } else if (request.uri == "switchUser") {
        std::string newUsername = payload.value("username", "");
        std::string newPassword = payload.value("password", "");

        if (session.switchUser(newUsername, newPassword)) {
            userIdToFd[session.getUserId()] = clientFd;
            userIdToFd.erase(oldId);

            json response = {{"userId", session.getUserId()},
                             {"username", session.getUsername()},
                             {"nickname", session.getNickname()}};

        } else if (request.uri == "/logout") {
        }
    }
}

void Server::persistMessage(const Message& msg) {
    this->messageDAO.insertMessage(msg);
}

void Server::closeClient(int clientFd) {
    auto it = userSessions.find(clientFd);
    if (it == userSessions.end())
        return;

    auto session = it->second;

    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);

    userSessions.erase(it);
}

bool Server::verifyToken(int clientFd, const std::string& _token) {
    auto it = userSessions.find(clientFd);
    if (it == userSessions.end()) {
        return false;
    }
    return it->second.getToken() == _token;
}
