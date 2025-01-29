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
#include <plog/Log.h>
#include <mutex>
#include <optional>
#include <memory>
#include "runnable.hpp"
#include "dataManager.hpp"
#include "threadPool.hpp"
#ifndef PROXY_NODE_H
#define PROXY_NODE_H

class ProxyNode : public Runnable {
public:
    ProxyNode(const std::string &name, const uint16_t &port, DataManager<std::string> *dataHandler) :
        m_name(name), m_port(port), m_dataHandler(dataHandler){};
    virtual ~ProxyNode(){};

    bool addHandler(const std::string &name);
    const std::string m_name;

protected:
    const uint16_t m_port;
    DataManager<std::string> *m_dataHandler;
};
#endif
