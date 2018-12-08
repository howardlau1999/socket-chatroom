#ifndef __USER_H_
#define __USER_H_

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include "Socket.h"

class ChatRoom;
class Server;
class User {
    static int count;
    int id;
    Socket sock;
    std::string nickname;
    std::shared_ptr<ChatRoom> chatroom;

   public:
    User(const std::string& nickname, const Socket& sock);
    void say(std::string message);
    void notifyMessage(std::shared_ptr<User> from, std::string const& message);
    void systemNotify(std::string const& message);
    void joinChatroom(int rid);
    int const& getId() const;
    Socket& getSocket();
    std::string const& getNickname() const;
};

#endif