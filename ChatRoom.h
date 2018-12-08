#ifndef __CHAT_ROOM_H_
#define __CHAT_ROOM_H_
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class User;
class ChatRoom {
    std::map<int, std::shared_ptr<User>> users;
    static int count;
    int id;
    std::string title;

   public:
    ChatRoom(std::string title);

    void postMessage(int uid, std::string const& message);

    void join(std::shared_ptr<User> user);
    void quit(std::shared_ptr<User> user);

    int const& getId() const;
    std::string const& getTitle() const;
};

#endif