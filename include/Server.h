// Server.hpp
#pragma once

#include <ChatRoom.h>
#include <DataAccessObject/ChatRoom.h>
#include <DataAccessObject/User.h>
#include <Session.h>
#include <TypeDef.h>

#include <cstdint>
#include <string>
#include <unordered_map>
#include "DataAccessObject/Message.h"

class Server {
   public:
    Server(uint16_t _port, UserDAO& _userDAO, ChatRoomDAO& _chatRoomDAO, MessageDAO& _messageDAO);
    ~Server();

    void start();
    void stop();

    void run();

   private:
    // Socket/epoll 管理
    int serverFd = -1;
    int epollFd = -1;
    uint16_t port;
    bool running = false;

    UserDAO& userDAO;
    ChatRoomDAO& chatRoomDAO;
    MessageDAO& messageDAO;

    std::unordered_map<int, Session> userSessions;  // key: clientFd
    std::unordered_map<int, ChatRoom> chatRooms;    // key: roomId

    void acceptNewClient();
    void closeClient(int clientFd);

    void handleClientMessage(int clientFd);
    bool isWebSocketUpgrade(const std::string& raw);
    void handleWebSocketHandshake(int clientFd, const std::string& raw);
    std::string recvWebSocketFrame(int clientFd);
    void sendWebSocketFrame(int clientFd, const std::string& payload,
                            uint8_t opcode = 0x1);
    std::string extractHeader(const std::string& raw, const std::string& name);
    std::string computeWebSocketAccept(const std::string& key);

    void routePrivateMessage(const Message& msg);
    void routeGroupMessage(const Message& msg);
    void handleSystemMessage(const Message& msg);
    void persistMessage(const Message& msg);
    std::string serialize(const Message& msg);
    Message parseMessage(const std::string& raw);
};
