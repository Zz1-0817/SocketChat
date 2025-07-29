## Chat Server Module

```cpp
  class ChatServer {
  private:
      int         serverFd;
      int         epollFd;
      std::vector<int> clients; // 活跃客户端列表
      uint16_t    port;

      void initServerSocket();
      void handleEvents();
      void handleNewConnection();
      void handleClientMessage(int clientFd);
      void broadcastMessage(const std::string &msg, int senderFd);

  public:
      explicit ChatServer(uint16_t port);
      ~ChatServer();

      void run();
  };
```

### Variables

- `serverFd`

  File description for socket.
  -1 denotes that NO socket is initialized.

- `epollFd`

### Methods

- `run()`

  Start the main loop.
  More specifically, this function does the followings:
  1. initial a socket server by `initServerSocket()`
  2. create an epoll instance by `epoll_create()`, and then add the socket server itself in the ready queue
  3. Main loop
    - Handle new connection by `handleNewConnection()`
    - Handle reading event or error event or hang up event
