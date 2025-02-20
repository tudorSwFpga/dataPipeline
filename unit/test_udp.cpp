// include doctest stuff
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include "doctest.h"
#include <memory>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <time.h>
#include <memory>
#include <thread>
#include <atomic>

// class under test
#include "../include/udp.hpp"
#include "../include/dataManager.hpp"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

TEST(UdpData, reception) {
    // UDPClient -> UDPServer -> DataManager -> Consumer
    spdlog::set_level(spdlog::level::debug);
    std::atomic<bool> stopTest(false);
    //instantiate simple datamanager to which udp server will push data
    DataManagerConf config{"testDataManager", 2, 2, BROADCAST};
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance(config);
    dataManager->setConsumer("UdpConsumer");
    //instantiate udp server and client
    const uint16_t udpPort = 50000 + rand() % 30;
    //udp client will push data to the server so there will be no data manager
    std::unique_ptr<UdpClient>client = std::make_unique<UdpClient>("udpclient_",udpPort,nullptr);
    UdpServer server("myUDPServer", udpPort, dataManager);
    EXPECT_EQ(server.m_name.compare("myUDPServer"), 0);
    // launch UDP server thread
    std::thread serverThread([&]() {
        while (!stopTest) {
            server.run();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            spdlog::debug("Server thread running");
        }
        return;
    });
    // launch data manager thread
    std::thread dmThread([dataManager]() {
            dataManager->run();
        });

    //let the client send some simple data
    client->send("Hello");
    client->send("World");
    sleep(1);
    //check received data
    std::vector<std::string> msgs;
    for (int i = 0; i < 2; i++) {
        dataManager->pop("UdpConsumer", msgs);
    }
    EXPECT_EQ(msgs.size(), 2);
    EXPECT_EQ(msgs.at(0),"Hello");
    EXPECT_EQ(msgs.at(1),"World");
    //stop threads and clean up
    stopTest = true;
    dataManager->stop();
    serverThread.join();
    dmThread.join();

}
