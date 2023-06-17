#define BACKLOG 50
#define MSG_MAX_SIZE 100 // 100 byts max sent by peer
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
#include "runnable.hpp"
#include "dataManager.hpp"
#include "proxyNode.hpp"
#ifndef TCP_H
#define TCP_H

class TcpServer:public ProxyNode
{
public:
	TcpServer(const std::string& name,const uint16_t& tcpPort,
		   DataManager<std::string>* dataHandler):
	ProxyNode(name,tcpPort,dataHandler),
	m_pollTimeout{1,1000000000}
	{
		this->m_dataHandler->setFeeder(name);
		m_sockfd = createSocket();
		if (m_sockfd < 0) {
        	throw std::runtime_error(std::strerror(errno));
    	}
    	PLOG_DEBUG << " TCP Server Ctor " << m_sockfd;

	}
	~TcpServer(){
		PLOG_DEBUG << " TCP Server Dtor";

	}	

	//TODO: why to declare this here since it should be inherited from Runnable
	void run();

private:
	std::list<int> m_connectedClientsFds;
	struct sockaddr_in m_Address;
	int m_sockfd;
	//create socket and bind to it; return its FD
	int createSocket();
	void manageConnections(); 
	//get connected clients list and convert to an array of pollfd. Used for poll syscall
	void fdToPollFdArray(pollfd* pollfds);
	void receiveData();
	std::string getPeerIp(sockaddr* addr);
    uint16_t getPeerPort(sockaddr* addr);
    std::mutex m_connectedClientsMutex;
	const timespec m_pollTimeout;
	//TODO: remove this from the constructor
};

#endif
