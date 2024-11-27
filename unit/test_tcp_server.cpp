// include doctest stuff
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include "doctest.h"
#include <memory>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <plog/Initializers/RollingFileInitializer.h>
#include <time.h>
#include <memory>
#include <thread>
#include <atomic>

// class under test
#include "../include/tcp.hpp"
#include "../include/dataManager.hpp"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

uint16_t getPortFromStr(std::string str) {
    int pos = str.find(":");
    std::string res;
    res = str.substr(pos + 1, str.length());
    return static_cast<uint16_t>(stoi(res));
}

bool isPortInList(std::list<std::string> connections, const uint16_t &port) {
    bool found;
    spdlog::debug("isPortInList, number of connections {0:d} ", connections.size());
    for (auto i : connections) {
        spdlog::debug("isPortInList {0} vs {1}", getPortFromStr(i), port);
        if (getPortFromStr(i) == port)
            found = true;
    }
    return found;
}

TEST(TCPConnection, 1client) {
    spdlog::set_level(spdlog::level::debug);
    std::atomic<bool> stopTest(false);
    srand(time(NULL));
    DataManagerConf config{"testDataManager", 2, 2, BROADCAST};
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance(config);
    // dataManager->setConf(2, 2, BROADCAST);
    const uint16_t tcpPort = 50000 + rand() % 30;
    TcpServer server("myTCP", tcpPort, dataManager);
    // launch TCP server thread
    std::thread serverThread([&]() {
        spdlog::debug("Server thread running");
        while (!stopTest) {
            server.run();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    // CHECK(server.m_name.compare("myTCP")==0);
    EXPECT_EQ(server.m_name.compare("myTCP"), 0);
    sleep(1);
    // create tcp client
    TcpClient client("myClient");
    EXPECT_EQ(client.connect(tcpPort), 0);
    sleep(1);

    std::string p = server.getConnectedPeers().front();
    getPortFromStr(server.getConnectedPeers().front());
    EXPECT_EQ(client.getMyPort(), getPortFromStr(server.getConnectedPeers().front()));

    std::cout << "STOP" << std::endl;
    stopTest = true;
    serverThread.join();
}

TEST(TCPConnection, 10clients) {
    std::atomic<bool> stopTest(false);
    srand(time(NULL));
    DataManagerConf config{"testDataManager", 2, 2, BROADCAST};
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance(config);
    // dataManager->setConf(2, 2, BROADCAST);
    const uint16_t tcpPort = 50000 + rand() % 30;
    TcpServer server("myTCP", tcpPort, dataManager);
    // launch TCP server thread
    std::thread serverThread([&]() {
        spdlog::debug("Server thread running");
        while (!stopTest) {
            server.run();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    EXPECT_EQ(server.m_name.compare("myTCP"), 0);
    sleep(1);

    std::array<std::unique_ptr<TcpClient>, 10> clientArr;
    for (int i = 0; i < clientArr.size(); i++) {
        clientArr.at(i) = std::make_unique<TcpClient>(std::string("client_" + std::to_string(i)));
        ;
        EXPECT_EQ(clientArr.at(i)->connect(tcpPort), 0);
        sleep(2);
    }
    // now check ports
    for (int i = 0; i < clientArr.size(); i++) {
        EXPECT_EQ((isPortInList(server.getConnectedPeers(), clientArr.at(i)->getMyPort())), true);
    }

    // one client disconnects
    // EXPECT_EQ(clientArr.at(1)->disconnect(),0);
    clientArr.at(1)->disconnect();
    clientArr.at(3)->disconnect();
    clientArr.at(5)->disconnect();
    sleep(5);
    EXPECT_EQ(server.getConnectedPeers().size(), 7);
    stopTest = true;
    serverThread.join();
}

TEST(TCPData, reception) {
    std::atomic<bool> stopTest(false);
    srand(time(NULL));
    DataManagerConf config{"testDataManager", 2, 2, BROADCAST};
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance(config);
    // dataManager->setConf(2, 2, BROADCAST);
    const uint16_t tcpPort = 50000 + rand() % 30;
    TcpServer server("myTCP", tcpPort, dataManager);
    // launch TCP server thread
    std::thread serverThread([&]() {
        spdlog::debug("Server thread running");
        while (!stopTest) {
            server.run();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    EXPECT_EQ(server.m_name.compare("myTCP"), 0);
    sleep(1);

    std::array<std::unique_ptr<TcpClient>, 3> clientArr;
    for (int i = 0; i < clientArr.size(); i++) {
        clientArr.at(i) = std::make_unique<TcpClient>(std::string("client_" + std::to_string(i)));
        ;
        EXPECT_EQ(clientArr.at(i)->connect(tcpPort), 0);
        sleep(2);
    }

    // one of clients sends data
    clientArr.at(1)->disconnect();
    clientArr.at(3)->disconnect();
    clientArr.at(5)->disconnect();
    sleep(5);
    EXPECT_EQ(server.getConnectedPeers().size(), 9);
    spdlog::debug("STOP");
    stopTest = true;
    serverThread.join();
}

/*
SUBCASE("Sending data") {
    std::array<std::shared_ptr<TcpClient>, 10> clientArr;
    for (int i=0; i<clientArr.size(); i++){
        std::shared_ptr<TcpClient> t = std::make_shared<TcpClient>(std::string("client_" + std::to_string(i)));
        clientArr.at(i)=t;
        CHECK(t->connect(tcpPort)==0);
    }
    //now check ports
    for (int i=0; i<clientArr.size(); i++){
        CHECK(isPortInList(server.getConnectedPeers(),clientArr.at(i)->getMyPort()));
    }*/

/*SUBCASE("Test create fb frame from spec") {

    Metavision::SimuEv::FbFrameSpec spec = {
        .batch_size = 1,
        .fb_frame_period_us = 33000, // fb frame period in us - typical: 33 ms
        .nb_frames = 5,            // number of fb frame to cover
        // first frame
        .ts_us = 0,                // timestamp of first frame
        .first_index = 10,         // start index
    };

    Metavision::SimuEv::FbFrames fbFrame = simuEv.create_fb_frames(spec);
}*/
