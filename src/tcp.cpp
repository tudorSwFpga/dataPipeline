#include <stdexcept>
#include <errno.h>
#include <cstring>
#include <chrono>
#include <poll.h>
#include <unistd.h>
#include <mutex>
#include <fcntl.h>
#include "../include/tcp.hpp"

/**
 * @return
 */
int TcpServer::createListenSocket() {
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    // init address to 0
    memset(&m_Address, 0, sizeof(sockaddr_in));
    // set ip socket type
    m_Address.sin_family = AF_INET;
    // bind port on any ip address
    m_Address.sin_port        = htons(m_port);
    m_Address.sin_addr.s_addr = inet_addr("127.0.0.1");
    if (bind(sockfd, (struct sockaddr *)&m_Address, sizeof(sockaddr_in)) < 0) {
        throw std::runtime_error(std::string("Cannot bind to asked port ") + std::string(std::strerror(errno)));
    }
    listen(sockfd, BACKLOG);
    spdlog::info(" Socket created; Listening on port: {} ", m_port);
    return sockfd;
}

void TcpServer::run() {
    acceptConnectionReq();
    checkRx();
}

/**
 *  Accept TCP Connections
 *  Query if there are clients that want to establish connection through accept syscall
 *  Store their File Descriptors in a vector
 */
void TcpServer::acceptConnectionReq() {
    spdlog::debug("TcpServer::acceptConnectionReq() \n");
    sockaddr *clientAddr = new sockaddr();
    socklen_t socketSize = sizeof(socklen_t);
    // Utils::setSyscallNonblock(m_sockfd);
    int flags = fcntl(m_sockfd, F_GETFL, 0);
    flags |= O_NONBLOCK;
    if (fcntl(m_sockfd, F_SETFL, flags) == -1) {
        spdlog::error("Error setting file descriptor to non-blocking");
    }
    const int fd = accept(m_sockfd, clientAddr, &socketSize);
    if (fd < 0)
        spdlog::debug(" No client connection request pending \n");
    else {
        std::lock_guard<std::mutex> lock(m_connectedClientsMutex);
        m_connectedClientsFds.push_back(fd);
        spdlog::info("Fd {} Accepted new connection with {} : {}", fd, getPeerIp(fd), getPeerPort(fd));
    }
    delete clientAddr;
}

/** TODO
 *  remove File descriptors from list when connections are closed
 *  close connections when TCP server is closed
 */
void TcpServer::remConnectedClient(int fd) {
    spdlog::debug("Removing client");
    std::lock_guard<std::mutex> lock(m_connectedClientsMutex);
    m_connectedClientsFds.remove(fd);
}

/* On success, poll() returns a nonnegative value which is the
       number of elements in the pollfds whose revents fields have been
       set to a nonzero value (indicating an event or an error).  A
       return value of zero indicates that the system call timed out
       before any file descriptors became read.
    */
int TcpServer::pollFds(pollfd *fds) {
    int ret;
    fdToPollFdArray(fds);
    ret = poll(fds, m_connectedClientsFds.size(), 1000);
    return ret;
}

bool TcpServer::hasPeerHungup(pollfd fd) {
    return fd.revents & (POLLHUP | POLLERR | POLLRDHUP);
}

bool TcpServer::hasReceivedData(pollfd fd) {
    return fd.revents & POLLIN;
}

void TcpServer::readRxData(pollfd fd) {
    char *msgBuf = new char[MSG_MAX_SIZE];
    auto msgSize = read(fd.fd, msgBuf, MSG_MAX_SIZE);
    std::string strippedMsg;
    for (int i = 0; i < msgSize; i++) {
        if (msgBuf[i] != '\0') {
            strippedMsg += msgBuf[i];
        }
    }
    if (msgSize != 0) {
        spdlog::info("TcpServer::readRxData From {} : {} ", fd.fd, strippedMsg);
        m_dataHandler->push(std::move(strippedMsg), m_name);
    }
    delete[] msgBuf;
}

/* Poll the list of sockets.
 */
void TcpServer::checkRx() {
    spdlog::debug("TcpServer::checkRx()");

    pollfd *pollfds = new pollfd[BACKLOG];
    int p           = pollFds(pollfds);
    if (p != 0) {
        for (int i = 0; i < m_connectedClientsFds.size(); i++) {
            if (hasPeerHungup(pollfds[i])) {
                spdlog::debug("Peer {} hang up", pollfds[i].fd);
                remConnectedClient(pollfds[i].fd);
            } else if (hasReceivedData(pollfds[i])) {
                spdlog::debug("Received data");
                readRxData(pollfds[i]);
            } else {
                spdlog::debug("Unknown event {}", pollfds[i].revents);
            }
        }
    }
    delete[] pollfds;
}

std::list<std::string> TcpServer::getConnectedPeers() {
    std::list<std::string> ret;
    std::string id;
    for (auto i : m_connectedClientsFds) {
        id = getPeerIp(i) + ":" + std::to_string(getPeerPort(i));
        ret.push_back(id);
    }
    return ret;
}

/**
 * @param: peer socket address
 * @return: string with its ipv4 address
 */
std::string TcpServer::getPeerIp(int fd) {
    struct sockaddr_in peerAddr;
    socklen_t socketSize = sizeof(peerAddr);
    ::getpeername(fd, (struct sockaddr *)&peerAddr, &socketSize);
    return std::string(inet_ntoa(peerAddr.sin_addr));
}

/**
 * @param
 * @return
 */
uint16_t TcpServer::getPeerPort(int fd) {
    struct sockaddr_in peerAddr;
    socklen_t socketSize = sizeof(peerAddr);
    ::getpeername(fd, (struct sockaddr *)&peerAddr, &socketSize);
    return peerAddr.sin_port;
}

void TcpServer::fdToPollFdArray(pollfd *pollfds) {
    int pos = 0;
    for (auto i : m_connectedClientsFds) {
        pollfds[pos].fd     = i;
        pollfds[pos].events = (POLLIN | POLLPRI | POLLERR | POLLRDHUP);
        /*this will poll for the following:
        #define POLLIN                0x001                 There is data to read.
        #define POLLPRI                0x002                 There is urgent data to read.
        */
        pos++;
    }
}
