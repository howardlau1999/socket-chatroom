#include <memory.h>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

#include <arpa/inet.h>
#include <curses.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/epoll.h>
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
int uid = -1;
int rid = -1;
int row, col;
int client_fd;
WINDOW *footer, *chatlist;
std::string nickname;
std::map<int, Protocol::ChatRoomData> chatrooms;

template <class... Args>
void printMessage(const char *format, Args... args) {
    wprintw(chatlist, format, args...);
    wrefresh(chatlist);
}

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

void prompt(int client_fd) {
    while (true) {
        wmove(footer, 0, 0);
        for (int i = 0; i < col; ++i) wprintw(footer, " ");
        wmove(footer, 0, 0);
        if (rid < 0)
            wprintw(footer, "%s> ", nickname.c_str());
        else
            wprintw(footer, "%s @ %s> ", nickname.c_str(),
                    chatrooms[rid].title.c_str());
        wrefresh(footer);
        char buf[100];
        wgetstr(footer, buf);

        if (rid < 0) {
            int join_rid = atoi(buf);
            joinChatroom(client_fd, join_rid);
        } else {
            sendMsg(client_fd, buf);
        }
    }
}

void processMessage(Socket &sock, const struct Protocol::MessageHeader header,
                    const char *data) {
    switch (header.type) {
        case Protocol::MessageHeader::MsgType::SET_UID:
            uid = *((int *)data);
            printMessage("client: set uid %d\n", uid);
            break;
        case Protocol::MessageHeader::MsgType::CHAT_MESSAGE:
            printMessage("%s\n", data);
            break;
        case Protocol::MessageHeader::MsgType::JOIN_CHATROOM:
            rid = *(int *)data;
            printMessage("You joined chatroom %s\n",
                         chatrooms[rid].title.c_str());
            break;
        case Protocol::MessageHeader::MsgType::CHATROOM_LIST:
            chatrooms = Protocol::unpackChatRoomList(data);
            printMessage("Chatroom List: \n");
            for (auto const &chatroom : chatrooms) {
                printMessage("%d: %s\n", chatroom.first,
                             chatroom.second.title.c_str());
            }
            break;
        default:
            printMessage("client: unknown message type %d\n", header.type);
    }
}

int main(int argc, char *argv[]) {
    int numbytes;
    char buf[BUFFER_LEN];
    struct addrinfo hints, *servinfo, *p;
    int rv;
    char s[INET6_ADDRSTRLEN];

    if (argc != 3) {
        cerr << "usage: client hostname nickname" << endl;
        exit(1);
    }

    nickname = argv[2];

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

    // prepare epoll
    int epoll_fd = epoll_create(2);
    if (epoll_fd == -1) {
        cerr << "client: failed to create epoll file descriptor" << endl;
        exit(1);
    }

    const int MAX_EVENTS = 10;
    const int EPOLL_TIMEOUT = 30000;
    struct epoll_event event, events[MAX_EVENTS];
    event.events = EPOLLIN;
    event.data.fd = client_fd;

    struct epoll_event stdin_event;
    stdin_event.events = EPOLLIN;
    stdin_event.data.fd = STDIN_FILENO;

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event)) {
        cerr << "client: failed to add network file descriptor to epoll"
             << endl;
        exit(1);
    }

    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, STDIN_FILENO, &stdin_event)) {
        cerr << "client: failed to add stdin file descriptor to epoll" << endl;
        exit(1);
    }

    initscr();
    clear();
    getmaxyx(stdscr, row, col);
    chatlist = subwin(stdscr, row - 1, col, 0, 0);
    scrollok(chatlist, true);
    footer = subwin(stdscr, 1, col, row - 1, 0);
    std::thread prompt_thread(prompt, client_fd);
    prompt_thread.detach();

    registerUser(client_fd, argv[2]);
    requestChatroomList(client_fd);

    while (true) {
        int event_count =
            epoll_wait(epoll_fd, events, MAX_EVENTS, EPOLL_TIMEOUT);
        for (int i = 0; i < event_count; ++i) {
            if (events[i].data.fd == STDIN_FILENO) continue;
            Socket sock(events[i].data.fd);
            struct Protocol::Message msg = sock.recvMessage();
            processMessage(sock, msg.header, msg.data);
            delete[] msg.data;
            refresh();
        }
    }

    close(epoll_fd);
    return 0;
}