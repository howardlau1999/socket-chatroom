#ifndef __SERVER_H_
#define __SERVER_H_

#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>

#include "Socket.h"

class ChatRoom;
class User;
class Server {
    std::map<int, std::shared_ptr<ChatRoom>> chatrooms;
    std::map<int, std::shared_ptr<User>> users, fd_map;
    Server() {}
    Server(const Server&) = delete;

   public:
    ~Server() = default;
    std::shared_ptr<User> getUserBySocket(const Socket& sock);
    std::shared_ptr<User> newUser(std::string const& nickname,
                                  const Socket& sock);
    std::shared_ptr<ChatRoom> newChatroom(std::string const& title);
    std::shared_ptr<ChatRoom> joinChatroom(int uid, int rid);
    void broadcastMessage(std::string const& message);
    void quitChatroom(int uid, int rid);
    void sendChatroomList(Socket& sock);
    static std::shared_ptr<Server> getInstance();
};

#endif