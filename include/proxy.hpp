#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdint.h>
#include <iostream>
#include <list>
#include <thread>
#include <time.h>
#include <poll.h>
#include <plog/Log.h>
#include <mutex>
#include <optional>
#include "runnable.hpp"
#include "dataManager.hpp"
#include "tcp.hpp"
#include "threadPool.hpp"
#include "runnable.hpp"
#include <optional>
#include <memory>
#ifndef PROXY_H
#define PROXY_H


class Proxy:public Runnable
{
public:

	Proxy(std::shared_ptr<ThreadPool> tp):m_tp(tp){
		PLOG_DEBUG << " Proxy Ctor";

	};
	~Proxy(){
		PLOG_DEBUG << " Proxy Dtor";

	};	

	enum ProxyType {
		TCP,
		UDP,
		CONSUMER,
		CUSTOM,
		UNKNOWN
	};

	

	bool addHandler(const std::string& name);
	bool addNode(ProxyType type,const uint16_t& port,const std::string& name);
	bool remNode(ProxyType type,const uint16_t& port,const std::string& name);

	void run();
	void stop();

	static ProxyType getProxyType(const std::string& type);



private:

	struct ProxyConf{
		ProxyType m_type;
		const uint16_t m_port;
		const std::string m_name;
	};

	DataManager<std::string>* m_dataHandler;
	std::vector<ProxyNode*> m_proxyNodeList;
	std::shared_ptr<ThreadPool> m_tp;
    std::mutex m_proxyListMutex;
};

#endif
