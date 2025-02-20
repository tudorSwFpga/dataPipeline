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
        spdlog::debug(" Consumer {} ctor", name);
    }

    ~Consumer() {
        spdlog::debug("{} Dtor", m_name);
    }

    bool run() override {
        m_isRunning = true;
        std::ofstream file(m_name + ".txt");
        if (!file.is_open()) {
            spdlog::error("Failed to open file");
            return false;
        }
        while (m_isRunning) {
            if (m_dataHandler->pop(m_name, m_data)) {
                for (auto &it : m_data) {
                    spdlog::debug("Consumer {} - {} ", m_name, it);
                    file << it << std::endl;
                }
                m_data.clear();
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        file.close();
        return true;
    }

    bool stop() override {
        m_isRunning = false;
        return true;
    }

private:
    std::vector<std::string> m_data;
};
