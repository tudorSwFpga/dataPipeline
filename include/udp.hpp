#include "spdlog/spdlog.h"
#include "proxyNode.hpp"
#include "dataManager.hpp"
#include <string>
#pragma once

class UdpServer : public ProxyNode {
public:
    UdpServer(const std::string &name, const uint16_t &tcpPort, DataManager<std::string> *dataHandler) :
        ProxyNode(name, tcpPort, dataHandler) {
        spdlog::debug("UDP Server ctor ");
        m_dataHandler->setFeeder(name);
        m_sockfd = createRxSocket();
    }
    ~UdpServer() {
        spdlog::debug("UDP Server dtor ");
    }
    // in running state all the magic operates
    bool run() override;
    std::string getName();

private:
    int m_sockfd;
    struct sockaddr_in m_Address;
    // data handler, used when receiving packets
    int createRxSocket();
};

class UdpClient : public ProxyNode {
public:
    UdpClient(const std::string &name, const uint16_t &rem_port, DataManager<std::string> *dataHandler) :
        ProxyNode(name, rem_port, dataHandler) {
        spdlog::debug("UDP Client ctor ");
        createTxSocket();
        // TODO: Better manage the return value
    }
    ~UdpClient() {
        spdlog::debug("UDP Client dtor ");
    }

    int send(const std::string &msg);

private:
    int m_sockfd;
    struct sockaddr_in m_address;
    void createTxSocket();
    bool run() override;
};
