#include "User.h"
#include "ChatRoom.h"
#include "Server.h"
#include "Protocol.h"

User::User(const std::string& nickname, const Socket& sock) : id(count++), nickname(nickname), sock(sock) {}
void User::say(std::string message) {
    chatroom->postMessage(this->id, message);
}

void User::notifyMessage(std::shared_ptr<User> from,
                         std::string const& message) {
    using Protocol::MessageHeader;
    std::string str = from->getNickname() + ": " + message;
    sock.sendMessage(MessageHeader::MsgType::CHAT_MESSAGE, str.c_str(), str.size() + 1);
}

void User::systemNotify(std::string const& message) {
    using Protocol::MessageHeader;
    sock.sendMessage(MessageHeader::MsgType::CHAT_MESSAGE, message.c_str(), message.size() + 1);
}

void User::joinChatroom(int rid) {
    chatroom = Server::getInstance()->joinChatroom(this->id, rid);
    using Protocol::MessageHeader;
    sock.sendMessage(MessageHeader::MsgType::JOIN_CHATROOM, (const char*)&rid, sizeof(rid));
}

int const& User::getId() const { return id; }

std::string const& User::getNickname() const { return nickname; }

Socket& User::getSocket() {
    return sock;
}

int User::count = 0;