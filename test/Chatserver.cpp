#include "ChatServer.h"

int main() {
    ChatServer server(8080); // 使用端口 8080 测试
    server.run();            // 启动服务器主循环
    return 0;
}

