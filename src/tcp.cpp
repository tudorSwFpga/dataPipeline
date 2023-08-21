#include <stdexcept>
#include <errno.h>
#include <cstring>
#include <chrono>
#include <poll.h>
#include <unistd.h>
#include <plog/Log.h>
#include "../include/tcp.hpp"

int TcpServer::createSocket(){
	PLOG_INFO << " Create socket ";

	int sockfd = socket(AF_INET, SOCK_STREAM, 0);
	PLOG_INFO << " Create socket 2, port " << m_port;

	if (sockfd < 0) {
        throw std::runtime_error(std::strerror(errno));
    }

	memset(&m_Address,0,sizeof(sockaddr_in)); //init address to 0
	m_Address.sin_family = AF_INET; //set ip socket
	m_Address.sin_port = htons(m_port);
	m_Address.sin_addr.s_addr = INADDR_ANY;

	if (bind(sockfd, (struct sockaddr *) &m_Address, sizeof(sockaddr_in)) < 0){
		throw std::runtime_error(std::string("Cannot bind to asked port ") + std::string(std::strerror(errno)));
	}

	listen(sockfd,BACKLOG);
	PLOG_INFO << " Socket created; Listening on port: " << m_port;


	return sockfd;
}

void TcpServer::run(){
	PLOG_DEBUG << " Running";
	manageConnections();
	receiveData();
}

/*  Manage TCP Connections
- accept clients that want to establish connection
- store their File Descriptors in a list
- remove File descriptors from list when connections are closed
- close connections when TCP server is closed
*/
void TcpServer::manageConnections(){
	PLOG_DEBUG << " Manage Connections";
	using namespace std::chrono_literals;
	sockaddr* clientAddr = new sockaddr();
	socklen_t socketSize = sizeof(socklen_t);
	int flags = fcntl(m_sockfd,F_GETFL,0);
	fcntl(m_sockfd, F_SETFL, flags | O_NONBLOCK);
	const int fd = accept(m_sockfd, clientAddr,&socketSize);
	if (fd < 0)
		PLOG_DEBUG << " No client connection request pending";
	else{
		PLOG_INFO << " Accepted new connection with : " <<  getPeerIp(clientAddr) << " : " << getPeerPort(clientAddr);
		std::lock_guard<std::mutex> guard(m_connectedClientsMutex);
		m_connectedClientsFds.push_back(fd);
	}
	delete clientAddr;
}

/* Poll the list of sockets.
*/
void TcpServer::receiveData(){
	//PLOG_DEBUG << "Receive data ";

	pollfd* pollfds = new pollfd[BACKLOG];
	char* msgBuf = new char[MSG_MAX_SIZE];
	int ret;
	fdToPollFdArray(pollfds);
	ret = poll(pollfds,m_connectedClientsFds.size(), 1000);
	/* On success, poll() returns a nonnegative value which is the
	   number of elements in the pollfds whose revents fields have been
	   set to a nonzero value (indicating an event or an error).  A
	   return value of zero indicates that the system call timed out
	   before any file descriptors became read.
	*/
	//PLOG_DEBUG << "Poll return " << ret;

    if (ret != 0) {
    	for (int i=0;i<m_connectedClientsFds.size();i++){
    		if (pollfds[i].revents & (POLLHUP | POLLERR)){ 
    		    PLOG_INFO << "Socket " << pollfds[i].fd << " has hung up ";
    			std::lock_guard<std::mutex> guard(m_connectedClientsMutex);
    			m_connectedClientsFds.remove(pollfds[i].fd);
    		}
    		   //call method that removes fd from list
    		else if (pollfds[i].revents & POLLIN ) {
    	       if (read(pollfds[i].fd, msgBuf, MSG_MAX_SIZE) != 0){
    	       	PLOG_INFO << "From " << pollfds[i].fd << std::string(msgBuf);
    	       	m_dataHandler->push(std::string(msgBuf),m_name);
    	       }
    	    } else {
    	    	PLOG_DEBUG << "on FD " << pollfds[i].fd << "Unknown poll revent: " << pollfds[i].revents;
    	    }
    	}
    } else {
    	PLOG_DEBUG << "Poll Time out on " << ret << " sockets";
    }
    delete[] msgBuf;
    delete[] pollfds;
}

std::string TcpServer::getPeerIp(sockaddr* addr){
	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
    return std::string(inet_ntoa(addr_in->sin_addr));
}

uint16_t TcpServer::getPeerPort(sockaddr* addr){
	struct sockaddr_in *addr_in = (struct sockaddr_in *)addr;
    return addr_in->sin_port;
}

void TcpServer::fdToPollFdArray(pollfd* pollfds ){
	int pos = 0;
	PLOG_DEBUG << "Having  " << m_connectedClientsFds.size() << " FDs";

	for (auto i:m_connectedClientsFds)
	{   
		pollfds[pos].fd    = i;
		pollfds[pos].events = (POLLIN | POLLPRI | POLLERR); 
		/*this will poll for the following:
		#define POLLIN                0x001                 There is data to read.  
		#define POLLPRI                0x002                 There is urgent data to read.  
		*/
		pos++;

	}
}