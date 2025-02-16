#include "../include/udp.hpp"
#include <stdexcept>
#include <errno.h>

int UdpServer::createRxSocket() {
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
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
    return sockfd;
}

void UdpServer::run() {
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));
    socklen_t addr_len = sizeof(m_Address);
    int n              = recvfrom(m_sockfd, buffer, sizeof(buffer), 0, (struct sockaddr *)&m_Address, &addr_len);
    if (n < 0) {
        spdlog::error("Error receiving data");
    } else {
        spdlog::debug("Received data: {}", buffer);
        m_dataHandler->push(std::string(buffer), m_name);
    }
}
