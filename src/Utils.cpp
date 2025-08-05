#include <TypeDef.h>
#include <Utils.h>

#include <chrono>
#include <json.hpp>
#include <sstream>
#include <string>
#include <iostream>

using json = nlohmann::json;

Method parseMethod(const std::string& str) {
    if (str == "GET")
        return Method::GET;
    if (str == "POST")
        return Method::POST;
    if (str == "PUT")
        return Method::PUT;
    if (str == "PATCH")
        return Method::PATCH;
    if (str == "DELETE")
        return Method::DELETE;
    if (str == "HEAD")
        return Method::HEAD;
    if (str == "OPTIONS")
        return Method::OPTIONS;
    if (str == "TRACE")
        return Method::TRACE;
    if (str == "CONNECT")
        return Method::CONNECT;
    return Method::UNKNOWN;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size())
        return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool start_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size())
        return false;
    return str.substr(0, suffix.size()) == suffix;
}

Request parseRequest(const std::string& raw) {
    Request request;

    if (raw.empty()) {
        return request;
    }

    size_t pos = raw.find("\r\n\r\n");
    std::string header_part = raw.substr(0, pos);
    request.body = raw.substr(pos + 4);

    std::istringstream header_stream(header_part);
    std::string request_line;
    std::getline(header_stream, request_line);
    std::istringstream request_line_stream(request_line);
    std::string _method;

    request_line_stream >> _method >> request.uri;
    request.method = parseMethod(_method);

    std::string line;
    while (std::getline(header_stream, line)) {
        if (line.empty() or line == "\r")
            continue;
        size_t sep = line.find(':');
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 1);

            // trim
            value.erase(0, value.find_first_not_of(" \r"));
            value.erase(value.find_last_not_of(" \r") + 1);
            request.headers[key] = value;
        }
    }
    return request;
}

Message parseMessage(const Request& request) {
    if (request.body.empty()) {
        return {};
    }
    json req = json::parse(request.body);

    Message msg;
    std::string typeStr = req.at("type").get<std::string>();
    if (typeStr == "Private") {
        msg.type = MessageType::Private;
    } else if (typeStr == "Group") {
        msg.type = MessageType::Group;
    } else if (typeStr == "System") {
        msg.type = MessageType::System;
    }

    msg.from = req.value("from", 0);
    msg.to = req.value("to", 0);
    msg.content = req.value("content", "");
    msg.timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
                        std::chrono::system_clock::now().time_since_epoch())
                        .count();

    return msg;
}

std::string msgToResponse(const Message& msg) {
    json j;
    j["from"] = msg.from;
    j["to"] = msg.to;
    j["content"] = msg.content;
    j["timestamp"] = msg.timestamp;

    switch (msg.type) {
        case MessageType::Private:
            j["type"] = "Private";
            break;
        case MessageType::Group:
            j["type"] = "Group";
            break;
        case MessageType::System:
            j["type"] = "System";
            break;
    }

    std::string content = j.dump();

    return "HTTP/1.1 200 OK\r\n"
           "Content-Type: application/json\r\n"
           "Content-Length: " +
           std::to_string(content.size()) +
           "\r\n"
           "Connection: keep-alive\r\n"
           "\r\n" +
           content;
}
