#include <plog/Log.h>
#ifndef CONSUMER_H
#define CONSUMER_H
#include <mutex>
#include "dataManager.hpp"
#include "proxyNode.hpp"
#include <thread>
#include <chrono>

class Consumer: public ProxyNode
{

public:
	//default constructor
	Consumer(const std::string& name,const uint16_t& port,
		   DataManager<std::string>* dataHandler):
	ProxyNode(name,port,dataHandler)
	{
		this->m_dataHandler->setConsumer(name);
        PLOG_DEBUG << " New consumer : " << name;
	}

	~Consumer(){
		PLOG_DEBUG << " Dtor : "	;
	}


	//bool addOnDataRxCb();

	void run(){
		while (m_isRunning){
			std::string data;
			if (m_dataHandler->pop(m_name,&data)){
				std::cout << "Consumer " << m_name <<  data << std::endl;
			} 
			//usleep(100000);
			//this_thread::sleep_for(chrono::milliseconds(100));
		}
	}	

	void start(){
		m_isRunning = true;
	}

	void stop(){
		m_isRunning = false;
	}

private:
	const std::string m_name;
	bool m_isRunning;
	
};

#endif

