#include <poll.h>
#include <string>
#include <dataManager.hpp>
#include "spdlog/spdlog.h"

template<class T>
DataManager<T> *DataManager<T>::getInstance(const std::string &name) {
    std::lock_guard<std::mutex> lock(m_Mutex);
    if (m_Pinstance == nullptr) {
        m_Pinstance = new DataManager<T>(name);
    }
    return m_Pinstance;
}

template<class T>
bool DataManager<T>::setConf(const uint8_t &inQueues, const uint8_t &outQueues, const MODE &mode) {
    m_maxNbInQueues  = inQueues;
    m_maxNbOutQueues = outQueues;
    m_mode           = mode;
    return true;
}

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
    m_isRunning = true;
    do {
        if (m_mode == BROADCAST) {
            DataManager::manageBroadcast();
        } else {
            DataManager::manageMap();
        }
    } while (m_isRunning);
    spdlog::debug(" DataManager stopped");
}

template<class T>
void DataManager<T>::stop() {
    spdlog::debug(" DataManager stopping...");
    m_isRunning = false;
    for (auto cv : m_condVarIn) {
        cv.notify_all();
    }
    for (auto &cv : m_condVarOut) {
        cv.notify_all();
    }
}
/*
check each input queue status and copy on every output queue
*/
template<class T>
void DataManager<T>::manageBroadcast() {
    spdlog::debug("DataManager::manageBroadcast Waiting...");
    std::unique_lock<std::mutex> lk(m_inQueuesMutex);
    m_condVarIn.wait(lk);
    T data;
    int i = 0;
    for (auto &it : m_inQueues) {
        spdlog::debug("manageBroadcast wtf is it happening");
        if (!it.empty()) {
            data = it.front();
            it.pop();
            { // local context to reduce lock time acquisition
                spdlog::debug(" Acquire out lock");
                std::unique_lock<std::mutex> lockout(m_outQueuesMutex);
                for (auto &it : m_outQueues) {
                    it.push(data);
                    spdlog::debug(" Pushing new data {} to output q;Notyifing consumers!", data);
                }
                m_condVarOut[i].notify_all();
                // TOOD: replace here by something that also notifies users
            }
        }
        i++;
    }
}

template<class T>
void DataManager<T>::manageMap() {}

template<class T>
void DataManager<T>::push(T &&data, const std::string &pushId) {
    // acquire mutex and push data into the input queue
    std::unique_lock<std::mutex> guard(m_inQueuesMutex);
    if (m_inQueuesIds.count(pushId) == 0) {
        spdlog::error("No input queue with ID: {}", pushId);
    } else {
        m_inQueues[m_inQueuesIds[pushId]].push(data);
        spdlog::info(" Pushed new data {} in queue {}", data, std::to_string(m_inQueuesIds[pushId]));
        // notify thread waiting waiting for input data
        m_condVarIn.notify_one();
    }
}

template<class T>
bool DataManager<T>::pop(const std::string &popId, std::vector<T> &data) {
    // acquire mutex and push data into the input queue
    if (m_outQueuesIds.count(popId) == 0) {
        spdlog::error("No output queue with ID: " + popId);
        return false;
    } else {
        std::unique_lock<std::mutex> lk(m_outQueuesMutex);
        const int id = m_outQueuesIds[popId];
        m_condVarOut[id].wait(lk);
        // pop oldest data and then erase it
        while (!m_outQueues[m_outQueuesIds[popId]].empty()) {
            data.push_back(m_outQueues[m_outQueuesIds[popId]].front());
            m_outQueues[m_outQueuesIds[popId]].pop();
        }
        spdlog::debug("Popping data from output queue {}", popId);
        return true;
    }
}

template<class T>
bool DataManager<T>::setFeeder(const std::string &appId) {
    if (m_inQueuesIds.count(appId) == 0) {
        // initially all the map is empty
        if (m_inQueuesIds.size() < m_maxNbInQueues) {
            m_inQueuesIds.emplace(std::make_pair(appId, m_inQueuesIds.size()));
            m_inQueues.push_back(std::queue<T>());
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
            m_outQueues.push_back(std::queue<T>());
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
