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
        //TODO push should return a bool that should be checked
        m_dataHandler->push(std::string(buffer), m_name);
    }
    //return true;
}

void UdpClient::createTxSocket() {
    m_sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (m_sockfd < 0) {
        throw std::runtime_error(std::strerror(errno));
    }

    // init address to 0
    memset(&m_address, 0, sizeof(sockaddr_in));
    // set ip socket type
    m_address.sin_family = AF_INET;
    // bind port on any ip address
    m_address.sin_port        = htons(m_port);
    m_address.sin_addr.s_addr = inet_addr("127.0.0.1");
}

void UdpClient::run(){
    std::vector<std::string> data;
    bool ret = true;
    if (m_dataHandler->pop(m_name, data)) {
        for (auto &it : data) {
            spdlog::debug("Udp Client {} - {} ", m_name, it);
            if (::sendto(m_sockfd, it.c_str(), it.size(), 0, (struct sockaddr *)&m_address, sizeof(sockaddr_in)) < 0) {
                spdlog::error("Error sending data");
                ret = false;
            } 
        }
        data.clear();
    }
    //return ret;
}
