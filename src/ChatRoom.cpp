#include "ChatRoom.h"
#include "User.h"

ChatRoom::ChatRoom(std::string title) : title(title), id(count++) {}

void ChatRoom::postMessage(int uid, std::string const& message) {
    for (auto& pair : users) {
        pair.second->notifyMessage(users[uid], message);
    }
}

void ChatRoom::join(std::shared_ptr<User> user) { users[user->getId()] = user; }
void ChatRoom::quit(std::shared_ptr<User> user) {
    users[user->getId()] = nullptr;
}

int const& ChatRoom::getId() const { return id; }
std::string const& ChatRoom::getTitle() const { return title; }

int ChatRoom::count = 0;