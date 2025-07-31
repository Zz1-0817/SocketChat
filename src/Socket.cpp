#include "../include/Socket.h"

Socket::Socket() : ip(""), port(0), fd(0)
{
    fd = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(fd < 0)printf("[error] : errno = %d errmsg = %s\n\n", errno, strerror(errno));
    else printf("create socket success!\n");
}

Socket::Socket(int sockfd) : ip(0), port(0)
{
    fd = sockfd;
}

Socket::~Socket()
{
    close(fd);
}

bool Socket::bind(const std::string &s_ip, uint16_t s_port)
{
    struct sockaddr_in sockaddr;
    std::memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    if(s_ip.empty())sockaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    else sockaddr.sin_addr.s_addr = inet_addr(s_ip.c_str());
    sockaddr.sin_port = htons(s_port);
    if(::bind(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)) < 0)
    {
        printf("bind[error] : errno = %d, errmsg = %s\n", errno, strerror(errno));
        return false;
    }
    else printf("socket bind success : ip = %s psrt = %d\n", s_ip.c_str(), s_port);
    ip = s_ip;
    port = s_port;
    return true;
}

bool Socket::listen(int backlog)
{
    if(::listen(fd, backlog) < 0)
    {
        printf("[error] : errno = %d, errmsg = %s\n", errno, strerror(errno));
        return false;
    }
    else printf("socket listening ...\n");
    return true;
}

bool Socket::connect(const std::string &s_ip, uint16_t s_port)
{
    struct sockaddr_in sockaddr;
    std::memset(&sockaddr, 0, sizeof(sockaddr));
    sockaddr.sin_family = AF_INET;
    sockaddr.sin_addr.s_addr = inet_addr(s_ip.c_str());
    sockaddr.sin_port = htons(s_port);

    if(::connect(fd, (struct sockaddr *) &sockaddr, sizeof(sockaddr)))
    {
        printf("[error]: errno = %d errmsg = %s\n", errno, strerror(errno));
        return false;
    }
    else printf("socket connect success!\n");
    return true;
}

int Socket::accept()
{
    int connfd = ::accept(fd, nullptr, nullptr);
    if(connfd < 0)
    {
        printf("[error] = %d errmsg = %s\n", errno, strerror(errno));
        return 0;
    }
    printf("socket accept : conn = %d\n", connfd);
    return connfd;
}

int Socket::send(int c_fd, const char *buf, int len)
{
    return ::send(c_fd, buf, len, 0);
}

int Socket::recv(int c_fd, char *buf, int len)
{
    return ::recv(c_fd, buf, len, 0);
}

void Socket::close(int s_fd)
{
    if(fd > 0)
    {
        ::close(s_fd);
        fd = 0;
    }
}

bool Socket::set_non_blocking()
{
    int flags = fcntl(fd, F_GETFL, 0);
    if(flags < 0)
    {
        printf("[error] : errno = %d errmsg = %s\n", errno, strerror(errno));
        return false;
    }
    flags |= O_NONBLOCK;
    if(fcntl(fd, F_SETFL, flags) < 0)
    {
        printf("[error] : errno = %d errmsg = %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::set_send_buffer(int size)
{
    int buff_size = size;
    if(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &buff_size, sizeof(buff_size)) < 0)
    {
        printf("[error] : errno = %d errnomsg = %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::set_recv_buffer(int size)
{
    int buff_size = size;
    if(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &buff_size, sizeof(buff_size)) < 0)
    {
        printf("[error] : errno = %d errnomsg = %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::set_linger(bool active, int seconds)
{
    struct linger l;
    memset(&l, 0, sizeof(l));
    l.l_onoff = active ? 1 : 0;
    l.l_linger = seconds;
    if(setsockopt(fd, SOL_SOCKET, SO_LINGER, &l, sizeof(l)) < 0)
    {
        printf("[error] : errno = %d errnomsg = %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::set_keepalive()
{
    int flag = 1;
    if(setsockopt(fd, SOL_SOCKET, SO_KEEPALIVE, &flag, sizeof(flag)) << 0)
    {
        printf("[error] : errno = %d errnomsg = %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}

bool Socket::set_reuseaddr()
{
    void *flag;
    if(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) < 0)
    {
        printf("[error] : errno = %d errnomsg = %s\n", errno, strerror(errno));
        return false;
    }
    return true;
}