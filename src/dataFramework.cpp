#include <sys/stat.h>
#include "../include/dataFramework.hpp"
#include <memory>
#include <unistd.h>

bool DataFramework::setConf(const std::string &conf) {
    struct stat buffer;
    // check if file exists with stat syscall
    if (stat(conf.c_str(), &buffer) != 0) {
        throw std::runtime_error("Configuration file " + conf + " does not exist");
    } else {
        m_config = conf;
        return this->parseConf();
    }
}

bool DataFramework::parseConf() {
    try {
        PLOG_DEBUG << "Parsing conf " << m_config;
        pt::read_json(m_config, m_pt);
        // create thread pool
        instantiateThreadPool();
        // create data manager
        if (!instantiateDataManager())
            throw std::runtime_error("No Data Manager instantiated!");
        // create input proxy, if any
        bool ret;
        ret = instantiateProxy("in");
        if (ret == false)
            throw std::runtime_error("No Input Proxy instantiated!");
        if (!instantiateProxy("out"))
            throw std::runtime_error("No Output Proxy instantiated!");
    }

    catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

// spread the jobs on N threads
void DataFramework::start() {
    m_threadPoolPtr->start();
    m_inProxyPtr->start();
    m_isRunning                          = true;
    std::function<void()> runDataManager = [this]() {
        while (m_isRunning) {
            m_dataManager->run();
        }
    };
    m_threadPoolPtr->QueueJob("DataManager", runDataManager);
}

// spread the jobs on N threads
void DataFramework::run() {
    if (m_isRunning) {
        m_inProxyPtr->run();
    }
}

void DataFramework::stop() {
    m_isRunning = false;
    m_dataManager->stop();
    m_threadPoolPtr->stop();
}

bool DataFramework::instantiateProxy(const std::string &direction) {
    bool ret = true;
    if (direction != "in" && direction != "out")
        return false;
    std::string proxyName = direction + "Proxy";
    auto pNode            = m_pt.get_child_optional(proxyName);
    if (pNode) {
        for (const auto &proxy : m_pt.get_child(proxyName)) {
            Proxy::ProxyType type = Proxy::getProxyType(proxy.second.get<std::string>("type"));
            std::string name      = proxy.second.get<std::string>("name");
            uint16_t port         = (uint16_t)proxy.second.get<int>("port");
            PLOG_DEBUG << "Add " << type << " proxy node, " << name;
            std::string dmname = m_dataManager->getName();
            if (direction == "in") {
                m_inProxyPtr = std::make_unique<Proxy>(m_threadPoolPtr);
                m_inProxyPtr->addHandler(m_dataManager->getName());
                if (!m_inProxyPtr->addNode(type, port, name))
                    ret = false;
            }
            /*if (direction == "out"){
                m_outProxyPtr = std::make_unique<Proxy>(m_threadPoolPtr);
                m_outProxyPtr->addHandler(m_dataManager->getName());
                if (!m_outProxyPtr->addNode(type,port,name)) ret = false;
            }*/
        }
    } else {
        ret = false;
    }
    return ret;
}

bool DataFramework::instantiateDataManager() {
    // create data manager
    bool ret                            = false;
    boost::optional<std::string> dmName = m_pt.get_optional<std::string>("dataManager.name");
    boost::optional<std::string> dmMode = m_pt.get_optional<std::string>("dataManager.mode");
    if (!dmName) {
        throw std::runtime_error("Specify dataManager.name parameter in topology file");
    }
    if (!dmMode) {
        throw std::runtime_error("Specify dataManager.mode parameter in topology file");
    }
    m_dataManager     = DataManager<std::string>::getInstance(*dmName);
    std::string dmode = *dmMode;
    if (dmode.compare("broadcast") == 0) {
        ret = m_dataManager->setConf(8, 8, BROADCAST);
        PLOG_DEBUG << "Instantiated Data Manager " << m_dataManager->getName();
    }
    return ret;
}

bool DataFramework::instantiateThreadPool() {
    boost::optional<int> threadCount = m_pt.get_optional<int>("dataManager.threadCount");
    threadCount ? m_threadCount = *threadCount : m_threadCount = 1;
    m_threadPoolPtr = std::make_shared<ThreadPool>(m_threadCount);
    PLOG_DEBUG << "Number of threads set to " << m_threadCount;
    return true;
}
