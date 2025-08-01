#pragma once

#include <cstdint>
#include <string>

enum class MessageType {
    Private,
    Group,
    System,
};

union Target {
    int receiverId;
    int roomId;
};

struct Message {
    MessageType type;
    int senderId;
    Target target;
    std::string content;
    uint64_t timestamp;
};

