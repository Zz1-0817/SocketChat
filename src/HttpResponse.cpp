#include "HttpRespond.h"
#include <iostream>
#include <string>
#include <sstream>
#include <fstream>

using namespace Http;

Method parse_method(const std::string& str) {
    if (str == "GET")     return Method::GET;
    if (str == "POST")    return Method::POST;
    if (str == "PUT")     return Method::PUT;
    if (str == "PATCH")   return Method::PATCH;
    if (str == "DELETE")  return Method::DELETE;
    if (str == "HEAD")    return Method::HEAD;
    if (str == "OPTIONS") return Method::OPTIONS;
    if (str == "TRACE")   return Method::TRACE;
    if (str == "CONNECT") return Method::CONNECT;
    return Method::GET;
}

bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool start_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.substr(0, suffix.size()) == suffix;
}

/*
 * Parsing HTTP responses. For example, an HTTP response has the form as following:
 *    POST /messages HTTP/1.1\r\n
 *    Host: localhost\r\n
 *    Content-Type: application/json\r\n
 *    Content-Length: 42\r\n
 *    \r\n
 *    {"user":"Adam","message":"Hello World!"}
 */
Response::Response(std::string raw) {
    if (raw.empty()) {
        std::cout << "Empty response!\n";
        return;
    }

    // Divide header and body
    size_t pos = raw.find("\r\n\r\n");
    std::string header_part = raw.substr(0, pos);
    this -> body = raw.substr(pos + 4);

    // Parsing header part
    std::istringstream header_stream(header_part);
    std::string request_line;
    std::getline(header_stream, request_line);
    std::istringstream request_line_stream(request_line);
    std::string _method;
    
    request_line_stream >> _method >> this->uri;
    this->method = parse_method(_method);

    std::string line;
    while (std::getline(header_stream, line)) {
        if (line.empty() or line == "\r") continue;
        size_t sep = line.find(':');
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 1);

            // truncate spaces
            value.erase(0, value.find_first_not_of(" \r"));
            value.erase(value.find_last_not_of(" \r") + 1);
            this -> headers[key] = value;
        }
    }
}

std::string Response::getContentType(const std::string& filename) {
    if (ends_with(filename, ".html")) return "text/html";
    if (ends_with(filename, ".css")) return "text/css";
    if (ends_with(filename, ".js")) return "application/javascript";
    if (ends_with(filename, ".png")) return "image/png";
    if (ends_with(filename, ".jpg") or ends_with(filename, ".jpeg")) return "image/jpeg";
    return "application/octet-stream";
}

std::string Response::readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return "";
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

std::string Response::handleGetRequest() {
    std::string filepath = this ->uri;
    if (start_with(filepath, "/")) filepath = filepath.substr(1); // 去掉开头的’/‘
    if (filepath.empty()) filepath = "index.html"; // 默认首页

    std::string file_content = readFile(filepath);

    if (!file_content.empty()) {
        std::string content_type = getContentType(filepath);
        std::ostringstream response;
        response << "HTTP/1.1 200 OK\r\n";
        response << "Content-Type: " << content_type << "\r\n";
        response << "Content-Length: " << file_content.size() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << file_content;
        return response.str();
    } else {
        std::string not_found = "<h1>404 Not Found</h1>";
        std::ostringstream response;
        response << "HTTP/1.1 404 Not Found\r\n";
        response << "Content-Type: text/html\r\n";
        response << "Content-Length: " << not_found.size() << "\r\n";
        response << "Connection: close\r\n\r\n";
        response << not_found;
        return response.str();
    }
}

std::string Response::httpRoute() // 路由处理
{
    if(this -> method == Method::GET) {
        return this->handleGetRequest();
    }
    else if(this -> method == Method::POST) {

        return  this->handlePostRequest();
    }
    else {
        std::ostringstream response;
        response << "HTTP/1.1 405 Method Not Allowed\r\n";
        response << "Content-Type: text/plain\r\n";
        response << "Content-Length: 0\r\n";
        response << "Connection: close\r\n\r\n";
        return response.str();
    }
}

std::string Response::handlePostRequest() {
    std::ostringstream response;
    std:: string response_body;
    if(this -> uri == "/messages") {
        size_t user_pos = this -> body.find("\"user\":\"");
        size_t msg_pos = this -> body.find("\"message\":\"");
        if (user_pos != std::string::npos && msg_pos != std::string::npos) {
            size_t user_start = user_pos + 8;
            size_t user_end = this -> body.find("\"", user_start);
            size_t msg_start = msg_pos + 11;
            size_t msg_end = this -> body.find("\"", msg_start);
            std::string user = this -> body.substr(user_start, user_end - user_start);
            std::string message = this -> body.substr(msg_start, msg_end - msg_start);
            response_body += "{\"status\":\"ok\", \"user\":\"" + user + "\", \"message\":\"" + message + "\"}";
        }
    }
    response << "HTTP/1.1 200 OK\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << response_body.size() << "\r\n";
    response << "Connection: close\r\n\r\n";
    response << response_body;
    return response.str();
}
