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
    pack_int(header.magic_start, (unsigned char*)buf);
    buf += 4;
    pack_int(header.data_len, (unsigned char*)buf);
    buf += 4;
    pack_int(header.type, (unsigned char*)buf);
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
    return title_total_len + (sizeof(int) * 2) * count + sizeof(int); // count
}

void Protocol::packChatRoomList(
    std::map<int, std::shared_ptr<ChatRoom>> const& chatrooms, char* buf) {
    pack_int(chatrooms.size(), (unsigned char*)buf);
    buf += 4;
    for (auto const& chatroom : chatrooms) {
        pack_int(chatroom.first, (unsigned char*)buf);
        buf += 4;
        int title_len = chatroom.second->getTitle().size() + 1;
        pack_int(title_len, (unsigned char*)buf);
        buf += 4;
        memcpy(buf, chatroom.second->getTitle().c_str(), title_len);
        buf += title_len;
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
        chatrooms[chatroom.rid ]=chatroom;
    }

    return std::move(chatrooms);
}