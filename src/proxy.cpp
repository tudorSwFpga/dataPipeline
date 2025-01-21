#include <stdexcept>
#include <errno.h>
#include <cstring>
#include <chrono>
#include <poll.h>
#include <unistd.h>
#include "spdlog/spdlog.h"
#include "../include/proxy.hpp"
#include "../include/proxyNode.hpp"
#include "../include/threadPool.hpp"
#include "../include/tcp.hpp"
#include "../include/runnable.hpp"
#include "../include/consumer.hpp"

// tcp proxy
bool Proxy::addNode(ProxyType type, const uint16_t &port, const std::string &name) {
    // create tcp server
    ProxyNode *node;
    switch (type) {
    case TCP:
        node = new TcpServer(name, port, m_dataHandler);
        break;
    case CONSUMER:
        node = new Consumer(name, port, m_dataHandler);
        break;
    default:
        throw std::runtime_error("Unknown type of node");
    }
    std::unique_lock<std::mutex> lk(m_proxyListMutex);
    m_proxyNodeList.push_back(node);
    spdlog::info("Added node {}, proxyNodeList size {}", name, m_proxyNodeList.size());

    return true;
}

void Proxy::addHandler(DataManager<std::string> *handler) {
    m_dataHandler = handler;
}

void Proxy::run() {
    spdlog::set_level(spdlog::level::debug);
    // spdlog::debug("Proy::run nodes {}",m_proxyNodeList.size());

    for (auto it : m_proxyNodeList) {
        std::function<void()> runProxy = [it]() { it->run(); };
        m_tp->QueueJob(it->m_name, runProxy);
        // spdlog::debug("Running proxy job {}",it->m_name);
    }
}

Proxy::ProxyType Proxy::getProxyType(const std::string &type) {
    if (type.compare("TCP") == 0) {
        return Proxy::ProxyType::TCP;
    } else if (type.compare("UDP") == 0) {
        return Proxy::ProxyType::UDP;
    } else if (type.compare("CUSTOM") == 0) {
        return Proxy::ProxyType::CUSTOM;
    } else if (type.compare("CONSUMER") == 0) {
        return Proxy::ProxyType::CONSUMER;
    } else {
        throw std::runtime_error("Unknown type of proxy, " + type);
    }
}
