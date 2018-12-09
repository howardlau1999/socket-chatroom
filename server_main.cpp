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
/* int getaddrinfo(const char *node,     // e.g. "www.example.com" or IP
                const char *service,  // e.g. "http" or port number
                const struct addrinfo *hints,
                struct addrinfo **res);
*/
/* int socket(int domain, int type, int protocol);  */
/* int connect(int sockfd, struct sockaddr *serv_addr, int addrlen); */
/* int listen(int sockfd, int backlog); */
/* int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);  */
/* int send(int sockfd, const void *msg, int len, int flags); */
/* int recv(int sockfd, void *buf, int len, int flags); */
/* close(sockfd); */

void *get_in_addr(struct sockaddr *sa) {
    if (sa->sa_family == AF_INET) {
        return &(((struct sockaddr_in *)sa)->sin_addr);
    }

    return &(((struct sockaddr_in6 *)sa)->sin6_addr);
}

void registerUser(Socket &sock, const char *nickname) {
    cout << "server: register user " << nickname << endl;
    auto user = Server::getInstance()->newUser(nickname, sock);
    user->getSocket().sendMessage(Protocol::MessageHeader::SET_UID,
                                  (const char *)&user->getId(),
                                  sizeof(user->getId()));
}

void processMessage(Socket &sock, const struct Protocol::MessageHeader header,
                    const char *data) {
    cout << "server: message type " << header.type << endl;
    switch (header.type) {
        case Protocol::MessageHeader::MsgType::REGISTER:
            registerUser(sock, data);
            break;
        case Protocol::MessageHeader::MsgType::CHAT_MESSAGE:
            Server::getInstance()->getUserBySocket(sock)->say(data);
            break;
        case Protocol::MessageHeader::MsgType::REQUEST_CHATROOM_LIST:
            Server::getInstance()->sendChatroomList(sock);
            break;
        case Protocol::MessageHeader::MsgType::JOIN_CHATROOM:
            Server::getInstance()->getUserBySocket(sock)->joinChatroom(
                *(int *)data);
            break;
        default:
            cout << "server: unknown message type " << header.type << endl;
    }
}

const int BACKLOG = 10;

void receiveData(Socket sock) {
    while (true) {
        struct Protocol::Message msg = sock.recvMessage();
        processMessage(sock, msg.header, msg.data);
        delete[] msg.data;
    }
}

int main() {
    int server_fd, new_fd;
    struct addrinfo hints, *servinfo, *p;
    struct sockaddr_storage clients_addr;
    socklen_t sin_size;
    struct sigaction sa;

    int yes = 1;
    char s[INET6_ADDRSTRLEN];
    int rv;

    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    if ((rv = getaddrinfo(NULL, SERVER_PORT, &hints, &servinfo)) != 0) {
        cerr << "getaddrinfo: " << gai_strerror(rv) << endl;
        return 1;
    }

    for (p = servinfo; p != NULL; p = p->ai_next) {
        if ((server_fd =
                 socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("server: socket error");
            continue;
        }

        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &yes,
                       sizeof(int)) == -1) {
            perror("server: setsockopt error");
            exit(1);
        }

        if (bind(server_fd, p->ai_addr, p->ai_addrlen) == -1) {
            close(server_fd);
            perror("server: bind error");
            continue;
        }

        break;
    }

    freeaddrinfo(servinfo);

    if (p == NULL) {
        cerr << "server: failed to bind" << endl;
        exit(1);
    }

    if (listen(server_fd, BACKLOG) == -1) {
        perror("server: listen");
        exit(1);
    }

    cout << "server: start listening on port " SERVER_PORT "..." << endl;
    Server::getInstance()->newChatroom("Chatroom A");
    Server::getInstance()->newChatroom("Chatroom B");
    while (true) {
        sin_size = sizeof clients_addr;
        new_fd = accept(server_fd, (struct sockaddr *)&clients_addr, &sin_size);

        if (new_fd == -1) {
            perror("server: accept");
            continue;
        }

        inet_ntop(clients_addr.ss_family,
                  get_in_addr((struct sockaddr *)&clients_addr), s, sizeof s);
        cout << "server: accepted connection from " << s << endl;
        std::thread recv_thread(receiveData, new_fd);
        recv_thread.detach();
    }

    return 0;
}