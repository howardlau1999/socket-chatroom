#include "Protocol.h"

#include <string.h>

void pack_int(unsigned long int i, unsigned char* buf) {
    *buf++ = i >> 24;
    *buf++ = i >> 16;
    *buf++ = i >> 8;
    *buf++ = i;
}

int unpack_int(unsigned char* buf) {
    unsigned long int i = ((unsigned long int)buf[0] << 24) |
                          ((unsigned long int)buf[1] << 16) |
                          ((unsigned long int)buf[2] << 8) | buf[3];
    return i;
}

void pack(unsigned char*) {}

void dopack(int i, unsigned char* buf) {
    *buf++ = i >> 24 & 0xFF;
    *buf++ = i >> 16 & 0xFF;
    *buf++ = i >> 8 & 0xFF;
    *buf++ = i & 0xFF;
}

template <class... Tail>
void pack(unsigned char*& buf, std::string str, Tail... tail) {
    memcpy(buf, str.c_str(), str.size() + 1);
    buf += str.size() + 1;
    pack(buf, tail...);
}

template <class Head, class... Tail>
void pack(unsigned char*& buf, Head head, Tail... tail) {
    dopack(head, buf);
    buf += sizeof(head);
    pack(buf, tail...);
}

template <class... Args>
void pack(char*& buf, Args... args) {
    pack((unsigned char*&)buf, args...);
}

struct Protocol::MessageHeader Protocol::buildHeader(
    Protocol::MessageHeader::MsgType type, const int data_len) {
    struct Protocol::MessageHeader header;
    header.magic_start = MAGIC_START;
    header.data_len = data_len;
    header.type = type;
    return header;
}

void Protocol::packHeader(const struct Protocol::MessageHeader& header,
                          char* buf) {
    pack(buf, header.magic_start, header.data_len, header.type);
}

struct Protocol::MessageHeader Protocol::unpackHeader(const char* buf) {
    struct Protocol::MessageHeader header;
    header.magic_start = unpack_int((unsigned char*)buf);
    buf += 4;
    header.data_len = unpack_int((unsigned char*)buf);
    buf += 4;
    header.type =
        (Protocol::MessageHeader::MsgType)unpack_int((unsigned char*)buf);
    return header;
}

int Protocol::calculatePackedRoomListSize(
    std::map<int, std::shared_ptr<ChatRoom>> const& chatrooms) {
    int count = chatrooms.size();
    int title_total_len = 0;
    for (auto const& chatroom : chatrooms) {
        title_total_len += chatroom.second->getTitle().size() + 1;
    }
    return title_total_len + (sizeof(int) * 2) * count + sizeof(int);  // count
}

void Protocol::packChatRoomList(
    std::map<int, std::shared_ptr<ChatRoom>> const& chatrooms, char* buf) {
    pack(buf, (int)chatrooms.size());
    for (auto const& chatroom : chatrooms) {
        int title_len = chatroom.second->getTitle().size() + 1;
        pack(buf, chatroom.first, title_len, chatroom.second->getTitle());
    }
}

std::map<int, Protocol::ChatRoomData> Protocol::unpackChatRoomList(
    const char* buf) {
    std::map<int, Protocol::ChatRoomData> chatrooms;
    int count = unpack_int((unsigned char*)buf);
    buf += 4;
    for (int i = 0; i < count; ++i) {
        Protocol::ChatRoomData chatroom;
        chatroom.rid = unpack_int((unsigned char*)buf);
        buf += 4;
        int title_len = unpack_int((unsigned char*)buf);
        buf += 4;
        while (title_len--) chatroom.title.push_back(*buf++);
        chatrooms[chatroom.rid] = chatroom;
    }

    return std::move(chatrooms);
}