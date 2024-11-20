#include <poll.h>
#include <string>
#include <dataManager.hpp>
#include "spdlog/spdlog.h"

template<class T>
DataManager<T> *DataManager<T>::getInstance(const std::string &name, uint8_t inQ, uint8_t outQ, MODE m) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Pinstance == nullptr) {
        m_Pinstance = new DataManager<T>(name, inQ, outQ, m);
    }
    return m_Pinstance;
}

/*template<class T>
bool DataManager<T>::setConf(const uint8_t &inQueues, const uint8_t &outQueues, const MODE &mode) {
    m_maxNbInQueues  = inQueues;
    m_maxNbOutQueues = outQueues;
    m_mode           = mode;
    return true;
}*/

template<class T>
void DataManager<T>::getConf(std::map<std::string, int> &inIds, std::map<std::string, int> &outIds) {
    inIds  = m_inQueuesIds;
    outIds = m_outQueuesIds;
}

template<class T>
void DataManager<T>::reset() {
    m_inQueues.erase(m_inQueues.begin(), m_inQueues.end());
    m_outQueues.erase(m_outQueues.begin(), m_outQueues.end());

    m_inQueuesIds  = std::map<std::string, int>();
    m_outQueuesIds = std::map<std::string, int>();
    spdlog::debug("Size = ", m_outQueues.size());
}

template<class T>
void DataManager<T>::run() {
    isRunning_ = true;
    do {
        if (m_mode == BROADCAST) {
            DataManager::manageBroadcast();
        } else {
            DataManager::manageMap();
        }
    } while (isRunning_);
    spdlog::debug(" DataManager stopped");
}

template<class T>
void DataManager<T>::stop() {
    spdlog::debug(" DataManager stopping...");
    isRunning_ = false;
    for (auto &q : m_inQueues) {
        q.stop();
    }
    for (auto &q : m_outQueues) {
        q.stop();
    }
}
/*
check each input queue status and copy on every output queue
*/
template<class T>
void DataManager<T>::manageBroadcast() {
    std::list<int> broadCastList;
    for (int i = 0; i < m_outQueues.size(); ++i) {
        broadCastList.push_back(i);
    }
    for (auto &it : m_inQueues) {
        if (!it.empty()) {
            spdlog::debug(" Broadcasting", it.queue.size());
            it.copy(m_outQueues, broadCastList);
        }
    }
}

template<class T>
void DataManager<T>::manageMap() {}

template<class T>
void DataManager<T>::push(T &&data, const std::string &pushId) {
    if (m_inQueuesIds.count(pushId) == 0) {
        spdlog::error("No input queue with ID: {}", pushId);
    } else {
        m_inQueues[m_inQueuesIds[pushId]].push(data);
        spdlog::info(" Pushed new data {} in queue {}", data, std::to_string(m_inQueuesIds[pushId]));
    }
}

template<class T>
bool DataManager<T>::pop(const std::string &popId, std::vector<T> &data) {
    T q_msg;
    if (m_outQueuesIds.count(popId) == 0) {
        spdlog::error("No output queue with ID: " + popId);
        return false;
    } else {
        const int id = m_outQueuesIds[popId];
        if (!m_outQueues[id].empty()) {
            spdlog::debug("Popping data from output queue {}, size {}", popId, m_outQueues[id].queue.size());
            m_outQueues[id].move(data);
            spdlog::debug("size is now {}", m_outQueues[id].queue.size());
        }
        return true;
    }
}

template<class T>
bool DataManager<T>::setFeeder(const std::string &appId) {
    if (m_inQueuesIds.count(appId) == 0) {
        // initially all the map is empty
        if (m_inQueuesIds.size() < m_maxNbInQueues) {
            m_inQueuesIds.emplace(std::make_pair(appId, m_inQueuesIds.size()));
            m_inQueues.emplace_back(Q<T>());
            spdlog::debug("New Feeder: {}", appId);

            // but once it becomes full, we can only find place if a feeder has left
        } else {
            auto f = m_inQueuesIds.find("empty");
            if (f != m_inQueuesIds.end()) {
                m_inQueuesIds.emplace(std::make_pair(appId, f->second));
                m_inQueuesIds.erase(f);
                spdlog::debug("New Feeder: {}", appId);
                return true;
            } else {
                spdlog::error(" No empty place to set new Feeder " + appId);
                return false;
            }
        }
    } else {
        spdlog::error(" Feeder already existing:  " + appId);
        return false;
    }
}

template<class T>
bool DataManager<T>::remFeeder(const std::string &appId) {
    if (m_inQueuesIds.count(appId) == 0) {
        return false;
    } else {
        auto f = m_inQueuesIds.find(appId);
        m_inQueuesIds.erase(f);
        return true;
    }
}

template<class T>
bool DataManager<T>::setConsumer(const std::string &appId) {
    if (m_outQueuesIds.count(appId) == 0) {
        // initially all the map is empty
        if (m_outQueuesIds.size() < m_maxNbOutQueues) {
            m_outQueuesIds.emplace(std::make_pair(appId, m_outQueuesIds.size()));
            // m_hasData.emplace(std::make_pair(appId,false));
            m_outQueues.emplace_back(Q<T>());
            spdlog::info("New Consumer " + appId);
            spdlog::info("New number of out queues {}", m_outQueues.size());
            // but once it becomes full, we can only find place if a feeder has left
        } else {
            auto f = m_outQueuesIds.find("empty");
            if (f != m_outQueuesIds.end()) {
                m_outQueuesIds.emplace(std::make_pair(appId, f->second));
                m_outQueuesIds.erase(f);
                spdlog::info("New Consumer 2 " + appId);
                return true;
            } else {
                spdlog::error("No empty place to set new Consumer " + appId);
                return false;
            }
        }
    } else {
        spdlog::error("Consumer already existing: " + appId);
        return false;
    }
}

template<class T>
std::string DataManager<T>::getName() {
    return m_Name;
}

template class DataManager<std::string>;
template<class T>
DataManager<T> *DataManager<T>::m_Pinstance{nullptr};
template<class T>
std::mutex DataManager<T>::m_Mutex;

// template class  DataManager<int>;
