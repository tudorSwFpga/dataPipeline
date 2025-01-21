#ifndef DATA_MANAGER_H
#define DATA_MANAGER_H
#include "spdlog/spdlog.h"
#include <queue>
#include <array>
#include <mutex>
#include <thread>
#include <map>
#include <condition_variable>
#include <functional>
#include <iterator>
#include "runnable.hpp"
#include "queue.hpp"
#include <string>
#include <algorithm>
#include <cctype>
/* This class helps managing data coming from one or several sources and copies it to one or several
   output queues.
*/

enum MODE { BROADCAST, MAP };

#define STRING_TO_CONF(str)                                                                                         \
    ([](std::string input) -> MODE {                                                                                \
        std::transform(input.begin(), input.end(), input.begin(), [](unsigned char c) { return std::tolower(c); }); \
        if (input == "broadcast")                                                                                   \
            return BROADCAST;                                                                                       \
        if (input == "map")                                                                                         \
            return MAP;                                                                                             \
        throw std::invalid_argument("Invalid mode: " + input);                                                      \
    }(str))

struct DataManagerConf {
    std::string name;
    uint8_t maxFeeders;
    uint8_t maxConsumers;
    MODE mode;
};

template<class T>
class DataManager {
protected:
    DataManager(const DataManagerConf &conf) :
        m_Name(conf.name), m_maxNbInQueues(conf.maxFeeders), m_maxNbOutQueues(conf.maxConsumers), m_mode(conf.mode) {
        spdlog::debug("DataManager Constructor;");
    }

    ~DataManager() {
        spdlog::debug(" DataM Destructor");
    }
    const std::string m_Name;

public:
    // singleton class is not clonable
    DataManager(DataManager &other) = delete;
    // singleton class is not assignable
    void operator=(const DataManager &) = delete;
    // get access to the instance

    static DataManager<T> *getInstance(const DataManagerConf &conf);

    // define number of input/output queues and the dispatching mode
    bool setConf(const uint8_t &inQueues, const uint8_t &outQueues, const MODE &mode);
    //
    void reset();
    // connect a feeder (get an id) to an input queue, if available
    bool setFeeder(const std::string &appId);
    // remove a feeder
    bool remFeeder(const std::string &appId);
    // push data to input queue of the feeder with pushId name
    // connect a consumer (get an id) to an output queue, if available
    bool setConsumer(const std::string &appId);
    // remove a consumer
    bool remConsumer(const std::string &appId);
    //
    void getConf(std::map<std::string, int> &inIds, std::map<std::string, int> &outIds);
    void push(T &&data, const std::string &pushId);
    // pop data from output queue of the consumer with popId nme
    bool pop(const std::string &popId, std::vector<T> &data);
    // get name
    std::string getName();

    // TODO:
    // setDecoder
    // setParser

    void run();
    void stop();

private:
    static DataManager<T> *m_Pinstance;
    static std::mutex m_Mutex;

    const uint8_t m_maxNbInQueues;
    const uint8_t m_maxNbOutQueues;
    const MODE m_mode;

    bool isRunning_;
    std::vector<Q<T>> m_inQueues;
    std::vector<Q<T>> m_outQueues;

    std::map<std::string, int> m_inQueuesIds;
    std::map<std::string, int> m_outQueuesIds;
    // have here a routing table variable or some mode

    // mutexes for protecting acess to input / output queues
    std::mutex m_inQueuesMutex;
    std::condition_variable m_condVarIn;
    std::mutex m_outQueuesMutex;
    // broadcast all the input data to output queues
    void manageBroadcast();
    // route input packets to output queues based on a routing table
    void manageMap();
};

#include "../src/dataManager.cpp"

#endif
