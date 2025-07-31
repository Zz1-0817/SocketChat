#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <iostream>
#include <fstream>

#define MAX_EVENTS 1024
#define PORT 18080

std::vector<std::string> messages;

// 设置非阻塞
int setnonblocking(int fd) {
    int opts = fcntl(fd, F_GETFL);
    if (opts < 0) return -1;
    return fcntl(fd, F_SETFL, opts | O_NONBLOCK);
}

// 简单 HTTP 响应
void send_response(int client_fd, const std::string& body, const std::string& content_type = "application/json") {
    std::string header = "HTTP/1.1 200 OK\r\nContent-Type: " + content_type +
                         "\r\nContent-Length: " + std::to_string(body.size()) +
                         "\r\nConnection: close\r\n\r\n";
    std::string response = header + body;
    send(client_fd, response.c_str(), response.size(), 0);
}

// 解析 POST body
std::string get_post_body(const std::string& req) {
    size_t pos = req.find("\r\n\r\n");
    if (pos == std::string::npos) return "";
    return req.substr(pos + 4);
}

void send_file_response(int client_fd, const std::string& filepath, const std::string& content_type) {
    std::ifstream fin(filepath, std::ios::binary);
    if (!fin) {
        send_response(client_fd, "<h1>404 Not Found</h1>", "text/html");
        return;
    }
    std::string body((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    send_response(client_fd, body, content_type);
}

// 处理请求
void handle_request(int client_fd, const std::string& req) {
    if (req.find("GET / ") == 0 || req.find("GET /chat_client.html") == 0) {
        // 主页或 html 文件
        send_file_response(client_fd, "chat_client.html", "text/html");
    }
    else if (req.find("GET /messages") == 0) {
        // 返回所有消息
        std::string body = "{\"list\":[";
        for (size_t i = 0; i < messages.size(); ++i) {
            if (i) body += ",";
            body += "\"" + messages[i] + "\"";
        }
        body += "]}";
        send_response(client_fd, body);
    } else if (req.find("POST /messages") == 0) {
        std::string body = get_post_body(req);
        // 这里简单取"user":"xxx","message":"yyy"
        size_t user_pos = body.find("\"user\":\"");
        size_t msg_pos = body.find("\"message\":\"");
        if (user_pos != std::string::npos && msg_pos != std::string::npos) {
            size_t user_start = user_pos + 8;
            size_t user_end = body.find("\"", user_start);
            size_t msg_start = msg_pos + 11;
            size_t msg_end = body.find("\"", msg_start);
            std::string user = body.substr(user_start, user_end - user_start);
            std::string message = body.substr(msg_start, msg_end - msg_start);
            messages.push_back(user + ": " + message);
        }
        send_response(client_fd, "{\"status\":\"ok\"}");
    } else {
        send_response(client_fd, "{\"error\":\"Not Found\"}", "application/json");
    }
}

int main() {
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    setnonblocking(listen_fd);

    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(PORT);

    bind(listen_fd, (sockaddr*)&serv_addr, sizeof(serv_addr));
    listen(listen_fd, SOMAXCONN);

    int epfd = epoll_create(MAX_EVENTS);
    epoll_event ev, events[MAX_EVENTS];
    ev.data.fd = listen_fd;
    ev.events = EPOLLIN;
    epoll_ctl(epfd, EPOLL_CTL_ADD, listen_fd, &ev);

    while (true) {
        int n = epoll_wait(epfd, events, MAX_EVENTS, -1);
        for (int i = 0; i < n; ++i) {
            int fd = events[i].data.fd;
            if (fd == listen_fd) {
                // 新连接
                sockaddr_in cli_addr;
                socklen_t cli_len = sizeof(cli_addr);
                int conn_fd = accept(listen_fd, (sockaddr*)&cli_addr, &cli_len);
                setnonblocking(conn_fd);
                ev.data.fd = conn_fd;
                ev.events = EPOLLIN;
                epoll_ctl(epfd, EPOLL_CTL_ADD, conn_fd, &ev);
            } else if (events[i].events & EPOLLIN) {
                char buf[4096] = {0};
                int len = recv(fd, buf, sizeof(buf), 0);
                if (len <= 0) {
                    close(fd);
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                } else {
                    std::string req(buf, len);
                    handle_request(fd, req);
                    close(fd); // 短连接
                    epoll_ctl(epfd, EPOLL_CTL_DEL, fd, nullptr);
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}