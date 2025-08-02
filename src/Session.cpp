#include <Session.h>

bool Session::login(const std::string &username, const std::string &password) {
    if(!this->userDAO.verifyPassword(username, password)) {
        return false;
    }

    auto userOpt = this->userDAO.getUserByUsername(username);
    if (!userOpt.has_value()) return false;

    const User &user = userOpt.value();
    this->userId = user.id;
    this->setNickname(user.nickname);
    this->setLoggedIn(true);
    auto roomIds = this->chatRoomDAO.getJoinedRoomIdsByUser(userId);
    for (int roomId : roomIds) {
        this->joinRoom(roomId);
    }

    userDAO.updateStatus(userId, 1);
    return true;
}

bool Session::switchUser(const std::string& newUsername, const std::string& newPassword) {
    if (!this->loggedIn) return false;
    int preUserId = this->userId;

    if(!this->login(newUsername, newPassword)) {
        return false;
    }

    this->userDAO.updateStatus(preUserId, 0);

    this->userDAO.updateStatus(this->userId, 1);
    return true;
}

int Session::getUserId() const {
    return this->userId;
}

int Session::getSocketFd() const {
    return this->socketFd;
}

std::string Session::getNickname() const {
    return this->nickname;
}

bool Session::isLoggedIn() const {
    return this->loggedIn;
}

bool Session::isDisconnected() const {
    return disconnected;
}

bool Session::isInRoom(int roomId) const {
    return this->joinedRoomIds.count(roomId) > 0;
}

void Session::joinRoom(int roomId) {
    this->joinedRoomIds.insert(roomId);
}

void Session::setLoggedIn(bool status) {
    this->loggedIn = status;
}

// TODO: 差一个数据库改名操作
void Session::setNickname(const std::string &name) {
    this->nickname = name;
}

void Session::markDisconnected() {
    this->disconnected = true;
}
