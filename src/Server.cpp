#include <Server.h>
#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <unistd.h>

#include <json.hpp>
#include <sstream>
#include <string>
#include "DataAccessObject/Message.h"

// OpenSSL for SHA1 & Base64
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>
#include <openssl/sha.h>

using json = nlohmann::json;

// Helpers --------------------------------------------------------------------
static int setNonBlocking(int fd) {
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags < 0)
        return -1;
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

// Server 实现 ---------------------------------------------------------------
Server::Server(uint16_t _port, UserDAO& _userDAO, ChatRoomDAO& _chatRoomDAO, MessageDAO& _messageDAO)
    : port(_port), userDAO(_userDAO), chatRoomDAO(_chatRoomDAO), messageDAO(_messageDAO) {}

Server::~Server() { stop(); }

void Server::start() {
    // Socket
    this->serverFd = ::socket(AF_INET, SOCK_STREAM, 0);
    if (serverFd < 0) {
        perror("Socket Error");
        exit(EXIT_FAILURE);
    }

    int opt = 1;
    setsockopt(serverFd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));


    //bind
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if(::bind(serverFd, (sockaddr*)&addr, sizeof(addr)) < 0 ) {
        perror("Bind Error");
        exit(EXIT_FAILURE);
    }

    if(::listen(serverFd, SOMAXCONN) < 0) {
        perror("Listen Error");
        exit(EXIT_FAILURE);
    }
    setNonBlocking(this->serverFd);

    epollFd = epoll_create1(0);
    epoll_event ev{};
    ev.events = EPOLLIN;
    ev.data.fd = this->serverFd;
    epoll_ctl(this->epollFd, EPOLL_CTL_ADD, serverFd, &ev);

    this->running = true;
}

void Server::stop() {
    running = false;
    if (serverFd >= 0) {
        close(serverFd);
        serverFd = -1;
    }
    if (epollFd >= 0) {
        close(epollFd);
        epollFd = -1;
    }
    for (auto& it : userSessions) {
        close(it.first);
    }
    userSessions.clear();
}

void Server::run() {
    const int MAX_EVENTS = 64;
    epoll_event events[MAX_EVENTS];

    while (running) {
        int n = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        if (n < 0 && errno == EINTR)
            continue;
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (fd == serverFd) {
                acceptNewClient();
            } else if (events[i].events & EPOLLIN) {
                handleClientMessage(fd);
            } else {
                closeClient(fd);
            }
        }
    }
}

void Server::acceptNewClient() {
    sockaddr_in cliAddr;
    socklen_t len = sizeof(cliAddr);
    int clientFd = ::accept(serverFd, (sockaddr*)&cliAddr, &len);
    if (clientFd < 0)
        return;
    setNonBlocking(clientFd);

    epoll_event ev{};
    ev.events = EPOLLIN | EPOLLET;
    ev.data.fd = clientFd;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, clientFd, &ev);

    Session session(clientFd, this->userDAO, this->chatRoomDAO);

    session.setLoggedIn(false);
    userSessions.insert({clientFd, std::move(session)});
}

void Server::closeClient(int clientFd) {
    epoll_ctl(epollFd, EPOLL_CTL_DEL, clientFd, nullptr);
    close(clientFd);
    userSessions.erase(clientFd);
}

void Server::handleClientMessage(int clientFd) {
    char buf[4096];
    ssize_t n = recv(clientFd, buf, sizeof(buf), 0);
    if (n <= 0) {
        closeClient(clientFd);
        return;
    }
    std::string raw(buf, n);
    auto& sess = userSessions[clientFd];

    if (!sess.websocket && isWebSocketUpgrade(raw)) {
        handleWebSocketHandshake(clientFd, raw);
        sess.websocket = true;
        return;
    }

    if (sess.websocket) {
        std::string payload = recvWebSocketFrame(clientFd);
        if (payload.empty()) {
            closeClient(clientFd);
            return;
        }

        Message msg = parseMessage(payload);
        switch (msg.type) {
            case MessageType::Private:
                routePrivateMessage(msg);
                break;
            case MessageType::Group:
                routeGroupMessage(msg);
                break;
            case MessageType::System:
                handleSystemMessage(msg);
                break;
        }
        persistMessage(msg);

        std::string out = serialize(msg);
        sendWebSocketFrame(clientFd, out);
        return;
    }

    closeClient(clientFd);
}

bool Server::isWebSocketUpgrade(const std::string& raw) {
    return raw.rfind("GET ", 0) == 0 &&
           raw.find("Upgrade: websocket") != std::string::npos;
}

void Server::handleWebSocketHandshake(int fd, const std::string& raw) {
    auto key = extractHeader(raw, "Sec-WebSocket-Key");
    auto accept = computeWebSocketAccept(key);

    std::ostringstream resp;
    resp << "HTTP/1.1 101 Switching Protocols\r\n"
         << "Upgrade: websocket\r\n"
         << "Connection: Upgrade\r\n"
         << "Sec-WebSocket-Accept: " << accept << "\r\n\r\n";

    auto s = resp.str();
    send(fd, s.data(), s.size(), 0);
}

std::string Server::recvWebSocketFrame(int fd) {
    uint8_t hdr[2];
    if (recv(fd, hdr, 2, MSG_WAITALL) != 2)
        return {};

    bool masked = hdr[1] & 0x80;
    uint64_t len = hdr[1] & 0x7F;
    if (len == 126) {
        uint8_t ext[2];
        recv(fd, ext, 2, MSG_WAITALL);
        len = (ext[0] << 8) | ext[1];
    } else if (len == 127) {
        uint8_t ext[8];
        recv(fd, ext, 8, MSG_WAITALL);
        len = 0;
        for (int i = 0; i < 8; ++i) len = (len << 8) | ext[i];
    }

    uint8_t mask[4] = {};
    if (masked)
        recv(fd, mask, 4, MSG_WAITALL);

    std::string data;
    data.resize(len);
    recv(fd, data.data(), len, MSG_WAITALL);

    if (masked) {
        for (uint64_t i = 0; i < len; ++i) {
            data[i] ^= mask[i % 4];
        }
    }
    return data;
}

void Server::sendWebSocketFrame(int fd, const std::string& payload,
                                uint8_t opcode) {
    std::vector<uint8_t> frame;
    frame.push_back(0x80 | (opcode & 0x0F));  // FIN + opcode

    size_t L = payload.size();
    if (L <= 125) {
        frame.push_back(static_cast<uint8_t>(L));
    } else if (L <= 0xFFFF) {
        frame.push_back(126);
        frame.push_back((L >> 8) & 0xFF);
        frame.push_back(L & 0xFF);
    } else {
        frame.push_back(127);
        for (int i = 7; i >= 0; --i) {
            frame.push_back((L >> (i * 8)) & 0xFF);
        }
    }

    frame.insert(frame.end(), payload.begin(), payload.end());
    send(fd, frame.data(), frame.size(), 0);
}

std::string Server::extractHeader(const std::string& raw,
                                  const std::string& name) {
    auto pos = raw.find(name + ": ");
    if (pos == std::string::npos)
        return "";
    auto eol = raw.find("\r\n", pos);
    return raw.substr(pos + name.size() + 2, eol - pos - name.size() - 2);
}

std::string Server::computeWebSocketAccept(const std::string& key) {
    std::string magic = key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    unsigned char sha1sum[SHA_DIGEST_LENGTH];
    SHA1(reinterpret_cast<const unsigned char*>(magic.c_str()), magic.size(),
         sha1sum);

    // Base64 编码
    BIO* bmem = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_push(BIO_new(BIO_f_base64()), bmem);
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    BIO_write(b64, sha1sum, SHA_DIGEST_LENGTH);
    BIO_flush(b64);

    BUF_MEM* bptr = nullptr;
    BIO_get_mem_ptr(b64, &bptr);
    std::string out(bptr->data, bptr->length);

    BIO_free_all(b64);
    return out;
}

void Server::routePrivateMessage(const Message& msg) {
    for (auto& [fd, sess] : userSessions) {
        if (sess.websocket && sess.getUserId() == msg.to) {
            sendWebSocketFrame(fd, serialize(msg));
            break;
        }
    }
}

void Server::routeGroupMessage(const Message& msg) {
    int roomId = msg.to;
    if (chatRooms.find(roomId) == chatRooms.end()) {
        auto optRoom = chatRoomDAO.getRoomById(roomId);
        if (!optRoom)
            return;
        chatRooms[roomId] = *optRoom;
    }
    const ChatRoom& room = chatRooms[roomId];

    for (int memberId : room.getMembers()) {
        for (auto& [fd, sess] : userSessions) {
            if (sess.websocket && sess.getUserId() == memberId) {
                sendWebSocketFrame(fd, serialize(msg));
            }
        }
    }
}

void Server::persistMessage(const Message& msg) {
    this->messageDAO.insertMessage(msg);
}

std::string Server::serialize(const Message& msg) {
    json j;
    j["type"] = static_cast<int>(msg.type);
    j["from"] = msg.from;
    j["to"] = msg.to;
    j["content"] = msg.content;
    return j.dump();
}

Message Server::parseMessage(const std::string& raw) {
    auto j = json::parse(raw);
    Message msg;
    msg.type = static_cast<MessageType>(j["type"].get<int>());
    msg.from = j["from"].get<int>();
    msg.to = j["to"].get<int>();
    msg.content = j["content"].get<std::string>();
    return msg;
}
