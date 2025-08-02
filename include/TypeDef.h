#pragma once

#include <cstdint>
#include <string>

enum class MessageType {
    Private,
    Group,
    System,
};

struct Message {
    MessageType type;
    int from;
    int to;
    std::string content;
    uint64_t timestamp;
};

struct User {
    int id;
    std::string username;
    std::string nickname;
};

