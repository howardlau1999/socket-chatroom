
#ifndef __SOCKET_H_
#define __SOCKET_H_
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

#include "Protocol.h"

class Socket {
    int fd;
    void sendAll(const char* buf, const int size);
    void recvAll(char* buf, const int size);
    Protocol::MessageHeader recvHeader();
public:
    Socket(const int fd);
    void sendMessage(Protocol::MessageHeader::MsgType type, const char* data, const int data_len);
    Protocol::Message recvMessage();
    const int getfd() const;
};

#endif // __SOCKET_H_

