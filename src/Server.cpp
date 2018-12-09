#include "Server.h"
#include "ChatRoom.h"
#include "User.h"

#include <deque>

std::shared_ptr<User> Server::newUser(std::string const& nickname, const Socket& sock) {
    auto user = std::shared_ptr<User>(new User(nickname, sock));
    return users[user->getId()] = fd_map[sock.getfd()] = user;
}

std::shared_ptr<User> Server::getUserBySocket(const Socket& sock) {
    if (fd_map.count(sock.getfd())) {
        return fd_map[sock.getfd()];
    } else {
        return nullptr;
    }
}

std::shared_ptr<ChatRoom> Server::newChatroom(std::string const& title) {
    auto chatroom = std::make_shared<ChatRoom>(title);
    return chatrooms[chatroom->getId()] = chatroom;
}

std::shared_ptr<ChatRoom> Server::joinChatroom(int uid, int rid) {
    chatrooms[rid]->join(users[uid]);
    return chatrooms[rid];
}

void Server::quitChatroom(int uid, int rid) {
    chatrooms[rid]->quit(users[uid]);
}

void Server::broadcastMessage(std::string const& message) {
    for (auto user : users) {
        user.second->systemNotify(message);
    }
}

void Server::sendChatroomList(Socket& sock) {
    int data_len = Protocol::calculatePackedRoomListSize(chatrooms);
    char* buf = new char[data_len];
    Protocol::packChatRoomList(chatrooms, buf);
    sock.sendMessage(Protocol::MessageHeader::MsgType::CHATROOM_LIST, buf, data_len);
    delete[] buf;
}

std::shared_ptr<Server> Server::getInstance() {
    static std::shared_ptr<Server> instance(new Server);
    return instance;
}

