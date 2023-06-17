#ifndef UDP_H
#define UDP_H

class UDPServer:public Runnable
{
public:
    UDPServer(uint16_t udpPort):m_udpPort(udpPort)
    {
        PLOG_INFO << " Creating object";
        createSocket();
    }
    ~UDPServer(){
    }   

    void setConf();
    void setHandler(std::shared_ptr<DataManager<std::string>> handler);

    //in running state all the magic operates
    void run();


private:

    //data handler, used when receiving packets
    std::shared_ptr<DataManager<std::string>> m_dataHandler;
    
    const uint16_t m_udpPort;
};

#endif
