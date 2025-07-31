#include<iostream>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<cstring>
#include<unistd.h>
#include<fcntl.h>
class Socket
{
    public:
        Socket();
        Socket(int sockfd);
        virtual ~Socket();
        bool listen(int backlog);
        bool bind(const std::string &s_ip, uint16_t s_port);
        bool connect(const std::string &s_ip, uint16_t s_port);
        int accept();
        int send(int c_fd, const char * buf, int len);
        int recv(int c_fd, char * buf, int len);
        void close(int s_fd);
        bool set_non_blocking();
        bool set_send_buffer(int size);
        bool set_recv_buffer(int size);
        bool set_linger(bool active, int seconds);
        bool set_keepalive();
        bool set_reuseaddr();
    protected:
        uint16_t port;  
        std::string ip;
        int fd;
};