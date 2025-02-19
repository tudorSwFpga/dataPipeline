#define BACKLOG 50
#define MSG_MAX_SIZE 512 // 512 bytes max sent by peer
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <iostream>
#include <list>
#include <thread>
#include <time.h>
#include <poll.h>
#include <mutex>
#include "runnable.hpp"
#include "dataManager.hpp"
#include "proxyNode.hpp"
#include <errno.h>
#include <unistd.h>
#include "spdlog/spdlog.h"

#ifndef TCP_H
#define TCP_H

class TcpServer : public ProxyNode {
public:
    TcpServer(const std::string &name, const uint16_t &tcpPort, DataManager<std::string> *dataHandler) :
        ProxyNode(name, tcpPort, dataHandler), m_name(name), m_pollTimeout{1, 1000000000} {
        this->m_dataHandler->setFeeder(name);
        m_sockfd = createListenSocket();
        spdlog::debug("TCP Server Ctor ");
    }
    ~TcpServer() {
        spdlog::debug("TCP Server Dtor ");
    }

    const std::string m_name;
    // TODO: why to declare this here since it should be inherited from Runnable
    bool run() override;

    std::list<std::string> getConnectedPeers();

private:
    std::list<int> m_connectedClientsFds;
    struct sockaddr_in m_Address;
    int m_sockfd;
    // create socket and bind to it; return its FD
    int createListenSocket();
    void acceptConnectionReq();
    void remConnectedClient(int fd);
    // get connected clients list and convert to an array of pollfd. Used for poll syscall
    void fdToPollFdArray(pollfd *pollfds);
    void checkRx();
    int pollFds(pollfd *fds);
    void readRxData(pollfd fd);
    bool hasPeerHungup(pollfd fd);
    bool hasReceivedData(pollfd fd);
    std::string getPeerIp(int fd);
    uint16_t getPeerPort(int fd);
    std::mutex m_connectedClientsMutex;
    const timespec m_pollTimeout;
    // TODO: remove this from the constructor
};

class TcpClient {
public:
    TcpClient(const std::string &name) : m_name(name) {
        spdlog::debug(" TCP Client Ctor {}", m_sockfd);
    }
    ~TcpClient() {
        spdlog::debug("TCP Client Dtor ");
    }

    const std::string m_name;

    int connect(const uint16_t &remPort) {
        struct sockaddr_in addr;
        int sockfd = socket(AF_INET, SOCK_STREAM, 0);
        if (sockfd < 0) {
            throw std::runtime_error(std::strerror(errno));
        }

        memset(&addr, 0, sizeof(sockaddr_in)); // init address to 0
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_family      = AF_INET;
        addr.sin_port        = htons(remPort);

        m_sockfd = sockfd;
        spdlog::debug("TcpClient {0} attempting connection to 127.0.0.1: {1} ", m_name, remPort);
        auto ret = ::connect(sockfd, (const struct sockaddr *)&addr, sizeof(sockaddr_in));
        return ret;
    }

    int send(const std::string &msg) {
        spdlog::debug("TcpClient sending data");
        return ::send(m_sockfd, msg.c_str(), sizeof(msg), 0);
    }

    int disconnect() {
        spdlog::debug("TcpClient disconnecting");
        return close(m_sockfd);
    }

    uint16_t getMyPort() {
        struct sockaddr_in peerAddr;
        socklen_t socketSize = sizeof(peerAddr);
        ::getsockname(m_sockfd, (struct sockaddr *)&peerAddr, &socketSize);
        return peerAddr.sin_port;
    }

    uint16_t getPeerPort() {
        struct sockaddr_in peerAddr;
        socklen_t socketSize = sizeof(peerAddr);
        ::getpeername(m_sockfd, (struct sockaddr *)&peerAddr, &socketSize);
        return peerAddr.sin_port;
    }

private:
    int m_sockfd;
};

#endif
