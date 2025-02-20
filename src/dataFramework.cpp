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
        pt::read_json(m_config, m_pt);
        // create thread pool
        instantiateThreadPool();
        // create data manager
        if (!instantiateDataManager())
            throw std::runtime_error("No Data Manager instantiated!");
        // create input proxy, if any
        if (instantiateProxy("out") == false)
            throw std::runtime_error("No Output Proxy instantiated!");
        if (instantiateProxy("in") == false)
            throw std::runtime_error("No Input Proxy instantiated!");
        spdlog::debug("DataFramework in proxy size {}", m_inProxyPtr->m_proxyNodeList.size());

    }

    catch (std::exception const &e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
}

void DataFramework::runJobs() {
    while (m_isRunning) {
        m_inProxyPtr->run();
    }
}

// spread the jobs on N threads
bool DataFramework::run() {
    m_threadPoolPtr->start();
    m_isRunning = true;
    m_threadPoolPtr->QueueJob("dm_run", [this]() { this->m_dataManager->run(); });
    m_outProxyPtr->run();
    m_mainThread = std::thread(&DataFramework::runJobs, this);
    return true;
}

bool DataFramework::stop() {
    m_isRunning = false;
    bool ret    = true;
    ret         = m_dataManager->stop() && m_inProxyPtr->stop() && m_outProxyPtr->stop();
    m_mainThread.join();
    m_threadPoolPtr->stop();
    return ret;
}

bool DataFramework::instantiateProxy(const std::string &direction) {
    bool ret = true;
    if (direction != "in" && direction != "out")
        return false;
    const std::string proxyName = direction + "Proxy";
    const std::string dmname    = m_dataManager->getName();

    auto pNode = m_pt.get_child_optional(proxyName);
    if (pNode) {
        if (direction == "in") {
            m_inProxyPtr = std::make_shared<Proxy>(m_threadPoolPtr);
            m_inProxyPtr->addHandler(m_dataManager);
        } else {
            m_outProxyPtr = std::make_shared<Proxy>(m_threadPoolPtr);
            m_outProxyPtr->addHandler(m_dataManager);
        }

        for (const auto &proxy : m_pt.get_child(proxyName)) {
            Proxy::ProxyType type = Proxy::getProxyType(proxy.second.get<std::string>("type"));
            std::string name      = proxy.second.get<std::string>("name");
            uint16_t port         = (uint16_t)proxy.second.get<int>("port");
            spdlog::debug("Add {} proxy node {}", type, name);
            if (direction == "in") {
                if (!m_inProxyPtr->addNode(type, port, name)) {
                    ret = false;
                }
            } else {
                if (!m_outProxyPtr->addNode(type, port, name)) {
                    ret = false;
                }
            }
        }
    } else {
        ret = false;
    }
    return ret;
}

bool DataFramework::setDataManagerConf() {
    bool ret                            = false;
    boost::optional<std::string> dmName = m_pt.get_optional<std::string>("dataManager.name");
    boost::optional<std::string> dmMode = m_pt.get_optional<std::string>("dataManager.mode");
    if (!dmName) {
        throw std::runtime_error("Specify dataManager.name parameter in topology file");
    }
    if (!dmMode) {
        throw std::runtime_error("Specify dataManager.mode parameter in topology file");
    }

    m_dataManagerConf.name         = *dmName;
    m_dataManagerConf.mode         = STRING_TO_CONF(*dmMode);
    m_dataManagerConf.maxFeeders   = 8;
    m_dataManagerConf.maxConsumers = 8;

    return true;
}

bool DataFramework::instantiateDataManager() {
    // create data manager
    setDataManagerConf();
    m_dataManager = DataManager<std::string>::getInstance(m_dataManagerConf);
    return true;
}

bool DataFramework::instantiateThreadPool() {
    boost::optional<int> threadCount = m_pt.get_optional<int>("dataManager.threadCount");
    threadCount ? m_threadCount = *threadCount : m_threadCount = 1;
    m_threadPoolPtr = std::make_shared<ThreadPool>(m_threadCount);
    spdlog::debug("Number of threads set to {}", m_threadCount);
    return true;
}

void DataFramework::getConsumerNames(std::vector<std::string> &names) {
    auto pNode = m_pt.get_child_optional("outProxy");
    if (pNode) {
        for (const auto &proxy : m_pt.get_child("outProxy")) {
            Proxy::ProxyType type = Proxy::getProxyType(proxy.second.get<std::string>("type"));
            if (type == Proxy::ProxyType::CONSUMER) {
                names.push_back(proxy.second.get<std::string>("name"));
            }
        }
    }
}
