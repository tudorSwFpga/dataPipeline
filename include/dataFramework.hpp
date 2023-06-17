#include <plog/Log.h>
#ifndef DATA_FRAMEWORK_H
#define DATA_FRAMEWORK_H
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


class DataFramework: public Runnable
{
public:
	//TODO: pass a config file 
	DataFramework(const std::string& conf) 
	{
		PLOG_DEBUG << "Constructor";
		if (this->setConf(conf)){
			PLOG_INFO << "Topology parsed and objects created; ready to start";
		} 
	}

	~DataFramework(){
		PLOG_DEBUG << "Dtor";
	}

	//set the path to the configuration file
	void start();
	void stop();
	void run();


private:

	pt::ptree m_pt;
	std::string m_config;
	int m_threadCount;
	bool setConf(const std::string& conf);
	bool parseConf();
	Proxy::ProxyType getProxyType(const std::string& type);
	
	std::shared_ptr<ThreadPool>  m_threadPoolPtr;
	std::unique_ptr<Proxy> m_inProxyPtr;
	std::unique_ptr<Proxy> m_outProxyPtr;
	DataManager<std::string>* m_dataManager;
	//Proxy m_inProxy(m_threadPoolPtr);
	//Proxy m_outProxy(m_threadPoolPtr);

};





#endif

