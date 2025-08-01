#include "http_response.h"

using namespace HttpR;

#include <iostream>
#include <string>
 
bool ends_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

bool start_with(const std::string& str, const std::string& suffix) {
    if (suffix.size() > str.size()) return false;
    return str.substr(0, suffix.size()) == suffix;
}

HttpResponse::HttpResponse(std::string HR)
{
    // 解析 HTTP 请求字符串 HR
    if (HR.empty()) {
        printf("http请求为空\n");
        return;
    }
    //分割HR为headers和body部分
    size_t pos = HR.find("\r\n\r\n");
    std::string header_part = HR.substr(0, pos);
    this -> body = HR.substr(pos + 4); // 4 是 \r\n\r\n 的长度
    std::istringstream header_stream(header_part);// 将头部部分转换为流
    std::string request_line;
    std::getline(header_stream, request_line); // 第一行为请求行
    std::istringstream request_line_stream(request_line);
    // 解析请求行
    request_line_stream >> this->method >> this->uri; // 获取请求方法与URI

    // 解析头部信息
    std::string line;
    while (std::getline(header_stream, line)) {
        if (line.empty() || line == "\r") continue;
        size_t sep = line.find(':');
        if (sep != std::string::npos) {
            std::string key = line.substr(0, sep);
            std::string value = line.substr(sep + 1);
            // 去除 value 前后的空格和回车

            value.erase(0, value.find_first_not_of(" \r"));
            value.erase(value.find_last_not_of(" \r") + 1);
            this -> headers[key] = value;
        }
    }
}

//返回参数中文件名的类型
std::string HttpResponse::GetContentType(const std::string& filename) {
    if (ends_with(filename, ".html")) return "text/html";
    if (ends_with(filename, ".css")) return "text/css";
    if (ends_with(filename, ".js")) return "application/javascript";
    if (ends_with(filename, ".png")) return "image/png";
    if (ends_with(filename, ".jpg") || ends_with(filename, ".jpeg")) return "image/jpeg";
    // 其他类型可按需添加
    return "application/octet-stream";
}

//读取文件内容
std::string HttpResponse::ReadFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) return "";
    std::ostringstream oss;
    oss << file.rdbuf();
    return oss.str();
}

//处理Get请求
//返回处理后的响应字符串
std::string HttpResponse::HandleGetRequest() {
    std::string filepath = this ->uri;
    if (start_with(filepath, "/")) filepath = filepath.substr(1); // 去掉开头的’/‘
    if (filepath.empty()) filepath = "index.html"; // 默认首页

    std::string file_content = ReadFile(filepath);

    if (!file_content.empty()) {
        std::string content_type = GetContentType(filepath);
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

std::string HttpResponse::HttepRoute() // 路由处理
{
    if(this -> method == "GET")return HandleGetRequest();
    else if(this -> method == "POST") return  HandlePostRequest();
    else {
        std::ostringstream response;
        response << "HTTP/1.1 405 Method Not Allowed\r\n";
        response << "Content-Type: text/plain\r\n";
        response << "Content-Length: 0\r\n";
        response << "Connection: close\r\n\r\n";
        return response.str();
    }
}

std::string HttpResponse::HandlePostRequest() {
    // 处理 POST 请求
    // 返回处理后的响应字符串
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