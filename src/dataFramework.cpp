#include <sys/stat.h>
#include "../include/dataFramework.hpp"
#include <memory>
#include <unistd.h>

bool DataFramework::setConf(const std::string& conf){
	struct stat buffer;
	//check if file exists with stat syscall
	if (stat(conf.c_str(), &buffer) != 0){
		throw std::runtime_error("Configuration file " + conf + " does not exist");
	} else {
        m_config = conf;
		return this->parseConf();
	}
}

bool DataFramework::parseConf(){
	try{
        PLOG_DEBUG << "Parsing conf " << m_config;
        pt::read_json(m_config, m_pt);

        boost::optional<int> threadCount = m_pt.get_optional<int>("dataManager.threadCount");
        threadCount ? m_threadCount = *threadCount : m_threadCount = 1;
        m_threadPoolPtr = std::make_shared<ThreadPool>(m_threadCount);
        PLOG_DEBUG << "Number of threads set to " << m_threadCount;

        //create data manager
        boost::optional<std::string> dmName = m_pt.get_optional<std::string>("dataManager.name");
        boost::optional<std::string> dmMode = m_pt.get_optional<std::string>("dataManager.mode");
        if (! dmName){
            throw std::runtime_error("Specify dataManager.name parameter in topology file");
        }
        if (! dmMode){
            throw std::runtime_error("Specify dataManager.mode parameter in topology file");
        }
        m_dataManager = DataManager<std::string>::getInstance(*dmName);
        std::string dmode = *dmMode;
        if (dmode.compare("broadcast") == 0){
	       m_dataManager->setConf(8, 8, BROADCAST); 
        }

        //create input proxy, if any
        auto inP = m_pt.get_child_optional("inProxy");
        if (inP){
            m_inProxyPtr = std::make_unique<Proxy>(m_threadPoolPtr);
            for (const auto &proxy : m_pt.get_child("inProxy")){
                m_inProxyPtr->addHandler(*dmName);
                Proxy::ProxyType type=Proxy::getProxyType(proxy.second.get<std::string>("type"));
                std::string name=proxy.second.get<std::string>("name");
                uint16_t port =(uint16_t)proxy.second.get<int>("port");
                PLOG_DEBUG << "Add in proxy node, " << name;

                m_inProxyPtr->addNode(type,port,name);
            }
        }

        //create output proxy, if any
        auto outP = m_pt.get_child_optional("outProxy");
        if (outP){
            m_outProxyPtr = std::make_unique<Proxy>(m_threadPoolPtr);
            for (const auto &proxy : m_pt.get_child("outProxy")){
                m_outProxyPtr->addHandler(*dmName);
                Proxy::ProxyType type=Proxy::getProxyType(proxy.second.get<std::string>("type"));
                std::string name=proxy.second.get<std::string>("name");
                uint16_t port =(uint16_t)proxy.second.get<int>("port");
                PLOG_DEBUG << "Add out proxy node, " << name;

                m_outProxyPtr->addNode(type,port,name);
            }
        }
    }
    
    catch (std::exception const& e) {
        std::cerr << e.what() << std::endl;
        return false;
    }
    return true;
    		
}

//spread the jobs on N threads
void DataFramework::start(){
    m_threadPoolPtr->start();
    m_inProxyPtr->start();
    m_isRunning = true;
    std::function<void()> runDataManager = [this]() {
        while (m_isRunning)
            {
                m_dataManager->run();
            }
    };
    m_threadPoolPtr->QueueJob("DataManager",runDataManager);
}


//spread the jobs on N threads
void DataFramework::run(){
    if (m_isRunning){
        m_inProxyPtr->run();
    }
}

void DataFramework::stop(){
        m_isRunning = false;
        m_dataManager -> stop();
        m_threadPoolPtr->stop();
}