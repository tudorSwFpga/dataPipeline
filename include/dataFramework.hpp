#ifndef DATA_FRAMEWORK_H
#define DATA_FRAMEWORK_H
#include "spdlog/spdlog.h"
#include "runnable.hpp"
#include "dataManager.hpp"
#include "proxy.hpp"
#include <string>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>
#include <boost/optional.hpp>
#include <optional>
#include <memory>
#include "threadPool.hpp"

/* This class helps managing data coming from one or several sources and copies it to one or several
   output queues.
*/

namespace pt = boost::property_tree;

class DataFramework : public Runnable {
public:
    // TODO: pass a config file
    DataFramework(const std::string &conf) {
        spdlog::debug(" DataFramework Ctor");
        if (this->setConf(conf)) {
            spdlog::info("Topology parsed and objects created; ready to start");
        }
    }

    ~DataFramework() {
        spdlog::debug(" DataFramework Dtor");
    }

    void stop() override;
    void run() override;

private:
    pt::ptree m_pt;
    std::string m_config;
    int m_threadCount;
    std::thread m_mainThread;
    bool setConf(const std::string &conf);
    bool setDataManagerConf();
    bool parseConf();
    bool instantiateProxy(const std::string &type);
    bool instantiateDataManager();
    bool instantiateThreadPool();
    void runJobs();
    DataManagerConf m_dataManagerConf;

    Proxy::ProxyType getProxyType(const std::string &type);

    std::shared_ptr<ThreadPool> m_threadPoolPtr;
    std::shared_ptr<Proxy> m_inProxyPtr;
    std::shared_ptr<Proxy> m_outProxyPtr;
    DataManager<std::string> *m_dataManager;
};

#endif
