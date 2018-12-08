#include <memory.h>
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
#include "ChatRoom.h"
#include "Protocol.h"
#include "Server.h"
#include "User.h"

#define SERVER_PORT "9315"
#define BUFFER_LEN 1024

using namespace std;
int uid;
std::map<int, Protocol::ChatRoomData> chatrooms;
void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void registerUser(Socket sock, std::string const &nickname) {
    sock.sendMessage(Protocol::MessageHeader::MsgType::REGISTER,
                     nickname.c_str(), nickname.size() + 1);
}

void sendMsg(Socket sock, std::string const &msg) {
    sock.sendMessage(Protocol::MessageHeader::MsgType::CHAT_MESSAGE,
                     msg.c_str(), msg.size() + 1);
}

void requestChatroomList(Socket sock) {
    char buf[1];
    sock.sendMessage(Protocol::MessageHeader::MsgType::REQUEST_CHATROOM_LIST,
                     buf, 1);
}

void joinChatroom(Socket sock, int rid) {
    sock.sendMessage(Protocol::MessageHeader::MsgType::JOIN_CHATROOM,
                     (const char *)&rid, sizeof(rid));
}

void processMessage(Socket &sock, const struct Protocol::MessageHeader header,
                    const char *data) {
    switch (header.type) {
        case Protocol::MessageHeader::MsgType::SET_UID:
            uid = *((int *)data);
            cout << "client: set uid " << uid << endl;
            break;
        case Protocol::MessageHeader::MsgType::CHAT_MESSAGE:
            cout << data << endl;
            break;
        case Protocol::MessageHeader::MsgType::JOIN_CHATROOM:
            cout << "you joined chatroom " << chatrooms[*(int *)data].title << endl;
            break;
        case Protocol::MessageHeader::MsgType::CHATROOM_LIST:
            chatrooms = Protocol::unpackChatRoomList(data);
            cout << "Chatroom List: " << endl;
            for (auto const &chatroom : chatrooms) {
                cout << chatroom.first << ": " << chatroom.second.title << endl;
            }
            break;
        default:
            cout << "client: unknown message type " << header.type << endl;
    }
}

void receiveData(Socket sock) {
    while (true) {
        struct Protocol::Message msg = sock.recvMessage();
        processMessage(sock, msg.header, msg.data);
        delete[] msg.data;
    }
}

int main(int argc, char *argv[]) {
    int client_fd, numbytes;
    char buf[BUFFER_LEN];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3) {
        cerr << "usage: client hostname nickname" << endl;
        exit(1);
    }

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    if ((rv = getaddrinfo(argv[1], SERVER_PORT, &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((client_fd =
                 socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket error");
            continue;
        }

        if (connect(client_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(client_fd);
            perror("client: connect error");
            continue;
        }

        break;
    }

    if (p == NULL) {
        cerr << "client: failed to connect" << endl;
        exit(2);
    }

    inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)&p->ai_addr), s,
              sizeof s);
    cout << "client: connected to " << s << endl;

    freeaddrinfo(servinfo);
    registerUser(client_fd, argv[2]);
    thread recv_thread(receiveData, client_fd);
    recv_thread.detach();

    requestChatroomList(client_fd);
    joinChatroom(client_fd, 0);

    while (true) {
        string s;
        getline(cin, s);
        printf("\33[2K\r");
        sendMsg(client_fd, s);
    }
    return 0;
}