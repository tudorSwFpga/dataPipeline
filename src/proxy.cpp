#include <stdexcept>
#include <errno.h>
#include <cstring>
#include <chrono>
#include <poll.h>
#include <unistd.h>
#include <plog/Log.h>
#include "../include/proxy.hpp"
#include "../include/proxyNode.hpp"
#include "../include/threadPool.hpp"
#include "../include/tcp.hpp"
#include "../include/runnable.hpp"

//tcp proxy
bool Proxy::addNode(ProxyType type,const uint16_t& port,const std::string& name){
	//create tcp server
	ProxyNode* node;
	if (type == TCP){
		node = new TcpServer(name,port,m_dataHandler);
	}
	std::unique_lock<std::mutex> lk(m_proxyListMutex);
	m_proxyNodeList.push_back(node);
	PLOG_DEBUG << "Added node " << name;
	return true;
}


bool Proxy::addHandler(const std::string& name){
	m_dataHandler = DataManager<std::string>::getInstance(name);
}

void Proxy::run(){
	//PLOG_DEBUG << "Proxy nodes " << m_proxyNodeList.size();
	for (auto it:m_proxyNodeList){
		std::function<void()> runProxy = [it](){ it->run();};
		m_tp->QueueJob(it->m_name,runProxy);
	//	PLOG_DEBUG << "Running proxy job " << it->m_name;
	}
	m_isRunning = true;
}

void Proxy::stop(){
	//TODO: do something here
}

Proxy::ProxyType Proxy::getProxyType(const std::string& type){
    if (type.compare("TCP") == 0){
        return Proxy::ProxyType::TCP;
    } else if (type.compare("UDP") == 0){
        return Proxy::ProxyType::UDP;
    } else if (type.compare("CUSTOM") == 0){
        return Proxy::ProxyType::CUSTOM;
    } else {
        throw std::runtime_error("Unknown type of proxy, " + type);
    }
}



