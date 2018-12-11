#include "Socket.h"

Socket::Socket(const int fd) : fd(fd) {} 

void Socket::setNonBlocking() {
    using namespace std;
    int opts;
    if ((opts = fcntl(fd, F_GETFL)) < 0) {
        cerr << "GETFL failed" << endl;
        exit(1);
    }
    opts = opts | O_NONBLOCK;
    if (fcntl(fd, F_SETFL, opts) < 0) {
        cerr << "SETFL failed" << endl;
        exit(1);
    }
}

void Socket::setAsync(int epoll_fd) {
    setNonBlocking();
    epoll_fd = epoll_fd;
}

void Socket::sendAll(const char* buf, const int size) {
    int sent = 0;
    while (sent < size) {
        int chunk = send(fd, buf, size - sent, 0);
        if (chunk == -1) {
            perror("send");
            return;
        }
        sent += chunk;
        buf += chunk;
    }
}

void Socket::recvAll(char* buf, const int size) {
    int received = 0;
    while (received < size) {
        int chunk = recv(fd, buf, size - received, 0);
        if (chunk == -1) {
            perror("recv");
            return;
        }
        received += chunk;
        buf += chunk;
    }
}

void Socket::sendMessage(Protocol::MessageHeader::MsgType type, const char* data, const int data_len) {
    struct Protocol::MessageHeader header = Protocol::buildHeader(type, data_len);
    char header_buf[Protocol::HEADER_SIZE];
    Protocol::packHeader(header, header_buf);
    sendAll(header_buf, Protocol::HEADER_SIZE);
    sendAll(data, data_len);
}

Protocol::MessageHeader Socket::recvHeader() {
    char buf[Protocol::HEADER_SIZE];
    recvAll(buf, Protocol::HEADER_SIZE);
    struct Protocol::MessageHeader header = Protocol::unpackHeader(buf);
    if (header.magic_start != MAGIC_START) {
        std::cerr << "malformed header received" << std::endl;
    }

    return header;
}

Protocol::Message Socket::recvMessage() {
    struct Protocol::MessageHeader header = recvHeader();
    char* buf = new char[header.data_len];
    recvAll(buf, header.data_len);
    struct Protocol::Message message;
    message.data = buf;
    message.header = header;
    return message;
}

const int Socket::getfd() const {
     return fd;
}