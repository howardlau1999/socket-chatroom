#ifndef __PROTOCOL_H_
#define __PROTOCOL_H_

#include <utility>
#include <deque>
#include <string>
#include <memory>

#include "ChatRoom.h"
const int MAGIC_START = 0xFF1999FF;

namespace Protocol {
struct MessageHeader {
    int magic_start;
    int data_len;
    enum MsgType {
        REGISTER,
        QUIT_CHATROOM,
        JOIN_CHATROOM,
        EXIT,
        REQUEST_CHATROOM_LIST,
        REQUEST_USER_LIST,
        SET_UID,
        CHATROOM_LIST,
        USER_LIST,
        CHAT_MESSAGE
    } type;
};

struct Message {
    struct MessageHeader header;
    char* data;
};

struct ChatRoomData {
    int rid;
    std::string title;
};

struct UserData {
    int uid;
    std::string nickname;
};


const int HEADER_SIZE =
    12;  // Because of padding, I have to hard-code the value
struct MessageHeader buildHeader(MessageHeader::MsgType type,
                                        const int data_len);
void packHeader(const struct MessageHeader& header, char* buf);
struct MessageHeader unpackHeader(const char* buf);

void packChatRoomList(std::map<int, std::shared_ptr<ChatRoom>> const& chatrooms, char* buf);
int calculatePackedRoomListSize(std::map<int, std::shared_ptr<ChatRoom>> const& chatrooms);
std::map<int, Protocol::ChatRoomData> unpackChatRoomList(const char* buf);
};  // namespace Protocol

#endif