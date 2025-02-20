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
#include <optional>
#include "runnable.hpp"
#include "dataManager.hpp"
#include "tcp.hpp"
#include "threadPool.hpp"
#include "runnable.hpp"
#include <optional>
#include <memory>
#pragma once

class Proxy : public Runnable {
public:
    Proxy(std::shared_ptr<ThreadPool> tp) : m_tp(tp) {
        spdlog::debug("Proxy ctor");
    };
    ~Proxy() {
        spdlog::debug("Proxy dtor");
        for (auto it : m_proxyNodeList) {
            delete it;
        }
    };

    enum ProxyType { TCPS, TCPC, UDPRX, UDPTX, CONSUMER, CUSTOM, UNKNOWN };

    void addHandler(DataManager<std::string> *handler);
    bool addNode(ProxyType type, const uint16_t &port, const std::string &name);
    bool remNode(ProxyType type, const uint16_t &port, const std::string &name);

    static ProxyType getProxyType(const std::string &type);
    bool run() override;
    bool stop() override;
    // todo move this to private
    std::vector<ProxyNode *> m_proxyNodeList;

private:
    struct ProxyConf {
        ProxyType m_type;
        const uint16_t m_port;
        const std::string m_name;
    };

    DataManager<std::string> *m_dataHandler;
    std::shared_ptr<ThreadPool> m_tp;
    std::mutex m_proxyListMutex;
};
