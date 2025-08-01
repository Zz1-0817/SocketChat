#pragma once
#include <chrono>
#include <string>

class Session {
   public:
    Session(int socketFd, int userId);

    int getUserId() const;
    int getSocketFd() const;

    bool isLoggedIn() const;
    void setLoggedIn(bool status);

    void updateLastActive();
    std::chrono::steady_clock::time_point getLastActiveTime() const;

    void setNickname(const std::string& name);
    std::string getNickname() const;

    bool isDisconnected() const;
    void markDisconnected();

   private:
    int socketFd;
    int userId;
    bool loggedIn;
    bool disconnected;

    std::chrono::steady_clock::time_point lastActiveTime;
    std::string nickname;
};
