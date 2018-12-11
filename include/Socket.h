
#ifndef __SOCKET_H_
#define __SOCKET_H_
#include <memory.h>
#include <iostream>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>
#include <fcntl.h>

#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <unistd.h>
#include <cstdlib>
#include <functional>
#include "Protocol.h"

class Socket {
    int fd, epoll_fd = -1;
    void sendAll(const char* buf, const int size);
    void recvAll(char* buf, const int size);
    void sendAllAsync(const char* buf, const int size);
    void recvAllAsync(char* buf, const int size);
    Protocol::MessageHeader recvHeader();
    void setNonBlocking();
public:
    Socket(const int fd);
    void sendMessage(Protocol::MessageHeader::MsgType type, const char* data, const int data_len);
    Protocol::Message recvMessage();
    void sendMessageAsync(Protocol::MessageHeader::MsgType type, const char* data, const int data_len, std::function<void()>);
    void recvMessageAsync(std::function<void(Protocol::Message)>);
    const int getfd() const;
    void setAsync(int epoll_fd);
};

#endif // __SOCKET_H_

