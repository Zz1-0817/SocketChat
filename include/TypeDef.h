#pragma once

#include <cstdint>
#include <string>
#include <map>

enum class MessageType {
    Private,
    Group,
    System,
};

enum class Method {
    GET,
    POST,
    PUT,
    PATCH,
    DELETE,
    HEAD,
    OPTIONS,
    TRACE,
    CONNECT,
    UNKNOWN
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

struct Request {
    Request(): method(Method::UNKNOWN) {};
    Method method;
    std::string uri;
    std::map<std::string, std::string> headers;
    std::string body;
};
