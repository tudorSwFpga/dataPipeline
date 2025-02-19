#include "spdlog/spdlog.h"
#include "proxyNode.hpp"
#include "dataManager.hpp"
#include <string>
#pragma once

class UdpServer : public ProxyNode {
public:
    UdpServer(const std::string &name, const uint16_t &udpPort, DataManager<std::string> *dataHandler) :
        ProxyNode(name, udpPort, dataHandler), m_name(name) {
        this->m_dataHandler->setFeeder(name);
        spdlog::debug("UDP Server ctor ");
        m_sockfd = createRxSocket();
    }
    ~UdpServer() {
        spdlog::debug("UDP Server dtor ");
    }
    // in running state all the magic operates
    bool run() override;

private:
    const std::string m_name;
    int m_sockfd;
    struct sockaddr_in m_Address;
    // data handler, used when receiving packets
    std::shared_ptr<DataManager<std::string>> m_dataHandler;
    int createRxSocket();
};

class UdpClient : public ProxyNode {
public:
    UdpClient(const std::string &name, const uint16_t& rem_port, DataManager<std::string> *dataHandler) : 
        ProxyNode(name, rem_port, dataHandler) {
        spdlog::debug("UDP Client ctor ");
        createTxSocket();
        //TODO: Better manage the return value
    }
    ~UdpClient() {
        spdlog::debug("UDP Client dtor ");
    }
private:
    int m_sockfd;
    struct sockaddr_in m_address;
    int send(const std::string &msg);
    void createTxSocket();
    bool run() override;
};
