#include <plog/Log.h>
#ifndef CONSUMER_H
#define CONSUMER_H
#include <mutex>
#include "dataManager.hpp"
#include <thread>
#include <chrono>

class Consumer: public ProxyNode
{

public:
	//default constructor
	Consumer(const std::string& name,const uint16_t& port,
		   DataManager<std::string>* dataHandler):
	ProxyNode(name,port,dataHandler),

	~Consumer(){
	}


	bool addOnDataRxCb();

	void print(){
		std::string data;
		while (m_isRunning){
			if (m_dataHandler->pop(m_name,&data)){
				std::cout << "Calculator " << m_name <<  data << std::endl;	
			} 
			usleep(100000);
			//this_thread::sleep_for(chrono::milliseconds(100));
		}
	}	

private:
	const std::string m_name;
	std::shared_ptr<dataManager<std::string>> m_dataHandler;

	
};

#endif

