#pragma once
#include <mutex>
#include "dataManager.hpp"
#include "proxyNode.hpp"
#include <thread>
#include <chrono>
#include <atomic>
#include <fstream>
#include "spdlog/spdlog.h"

class Consumer : public ProxyNode {
public:
    // default constructor
    Consumer(const std::string &name, const uint16_t &port, DataManager<std::string> *dataHandler) :
        ProxyNode(name, port, dataHandler) {
        this->m_dataHandler->setConsumer(name);
        spdlog::debug(" New consumer : {} ", name);
    }

    ~Consumer() {
        spdlog::debug("{} Dtor", m_name);
    }

    void run() {
        m_isRunning = true;
        std::ofstream file(m_name + ".txt");
        if (!file.is_open()) {
            spdlog::error("Failed to open file");
            return;
        }
        while (m_isRunning) {
            if (m_dataHandler->pop(m_name, m_data)) {
                for (auto &it : m_data) {
                    spdlog::debug("Consumer {} - {} ", m_name, it);
                    file << it << std::endl;
                }
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        file.close();
    }

    void stop() {
        m_isRunning = false;
    }

private:
    std::vector<std::string> m_data;
};
