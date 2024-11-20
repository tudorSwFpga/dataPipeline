// include doctest stuff
// #define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
// #include "doctest.h"
#include <memory>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <plog/Log.h>
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

std::string gen_random(const int len) {
    static const char alphanum[] = "0123456789"
                                   "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                   "abcdefghijklmnopqrstuvwxyz";
    std::string tmp_s;
    tmp_s.reserve(len);

    for (int i = 0; i < len; ++i) {
        tmp_s += alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    return tmp_s;
}

std::thread run_consumer(const std::string &id, DataManager<std::string> *dataManager,
                         std::vector<std::string> &rx_data, std::atomic<bool> &stop) {
    std::thread consumerThread([&id, &stop, &rx_data, dataManager]() {
        spdlog::debug("consumer thread running");
        std::vector<std::string> msgs;
        while (!stop) {
            dataManager->pop(id, msgs);
            rx_data.insert(rx_data.end(), msgs.begin(), msgs.end());
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        std::cout << id << " stopped" << std::endl;
    });
    return consumerThread;
}

std::thread run_feeder(const std::string &id, DataManager<std::string> *dataManager, std::vector<std::string> &tx_data,
                       std::atomic<bool> &stop) {
    spdlog::debug("run feeder: " + id);
    std::thread feederThread([dataManager, id, &stop, &tx_data]() {
        spdlog::debug("feeder {} thread running", id);
        while (!stop) {
            std::string msg = gen_random(16);
            tx_data.push_back(msg);
            dataManager->push(std::move(msg), id);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
        std::cout << id << " stopped" << std::endl;
    });

    return feederThread;
}

std::thread run_data_manager(DataManager<std::string> *dataManager) {
    std::thread dataThread([dataManager]() {
        spdlog::debug("data manager thread running");
        dataManager->run();
    });
    return dataThread;
}

TEST(DataManager, FeedersAndConsumers) {
    spdlog::set_level(spdlog::level::debug);
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance("testDataManager", 2, 2, BROADCAST);
    // dataManager->setConf(2, 2, BROADCAST);
    dataManager->setFeeder("Feeder0");
    dataManager->setFeeder("Feeder3");
    dataManager->setFeeder("Feeder1");
    dataManager->setConsumer("Consumer2");
    dataManager->setConsumer("Consumer0");
    dataManager->setConsumer("Consumer1");
    std::map<std::string, int> inIds, outIds;
    dataManager->getConf(inIds, outIds);
    EXPECT_EQ(inIds.size(), 2);
    EXPECT_EQ(inIds.count("Feeder0"), 1);
    EXPECT_EQ(inIds.count("Feeder3"), 1);
    EXPECT_EQ(inIds.count("Feeder1"), 0);
    EXPECT_EQ(outIds.count("Consumer2"), 1);
    EXPECT_EQ(outIds.count("Consumer0"), 1);
    EXPECT_EQ(outIds.count("Consumer1"), 0);
    dataManager->reset();
    dataManager->getConf(inIds, outIds);
    EXPECT_EQ(inIds.count("Feeder0"), 0);
    EXPECT_EQ(inIds.count("Feeder3"), 0);
    dataManager->setFeeder("Feeder0");
    dataManager->setFeeder("Feeder0");
    dataManager->getConf(inIds, outIds);
    EXPECT_EQ(inIds.size(), 1);
    EXPECT_EQ(inIds.count("Feeder0"), 1);
    dataManager->remFeeder("Feeder0");
    dataManager->getConf(inIds, outIds);
    EXPECT_EQ(inIds.count("Feeder0"), 0);
}

TEST(DataManager, 2Feeders1Consumer) {
    spdlog::set_level(spdlog::level::debug);
    std::vector<std::string> tx_data[2];
    std::atomic<bool> stopFeeders(false);
    std::atomic<bool> stopConsumers(false);
    srand(time(NULL));
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance("testDataManager", 2, 2, BROADCAST);
    // dataManager->setConf(2, 2, BROADCAST);
    dataManager->setFeeder("Feeder0");
    dataManager->setFeeder("Feeder1");
    dataManager->setConsumer("Consumer1");
    // data manager thread
    // launch 1 thread per feeder
    std::thread t[4];
    for (int i = 0; i < 2; i++) {
        const std::string id = "Feeder" + std::to_string(i);
        t[i]                 = run_feeder(id, dataManager, tx_data[i], stopFeeders);
    }
    // consumer thread
    std::vector<std::string> rx_data;
    t[2] = run_consumer("Consumer1", dataManager, rx_data, stopConsumers);
    t[3] = run_data_manager(dataManager);
    sleep(5);
    std::cout << "STOP" << std::endl;
    stopFeeders = true;
    sleep(1);
    dataManager->stop();
    stopConsumers = true;
    for (int i = 0; i < 4; i++) {
        t[i].join();
    }

    // check tx vs rx data
    for (int i = 0; i < 2; i++) {
        for (auto it : tx_data[i]) {
            bool found = false;
            // check each tx is in rx
            for (auto itrx : rx_data) {
                if (it.compare(itrx) == 0)
                    found = true;
            }
            if (found) {
                std::cout << "Tx: " << it << " FOUND" << std::endl;
            } else {
                std::cout << "Tx: " << it << " KO" << std::endl;
            }
            EXPECT_EQ(found, true);
        }
    }
}

TEST(DataManager, 1Feeder2Consumers) {
    spdlog::set_level(spdlog::level::debug);
    std::map<std::string, int> inIds, outIds;
    std::vector<std::string> tx_data;
    std::vector<std::string> rx_data[2];
    std::atomic<bool> stopTest(false);
    srand(time(NULL));
    DataManager<std::string> *dataManager = DataManager<std::string>::getInstance("testDataManager", 2, 2, BROADCAST);
    // reset data manager because singleton
    dataManager->reset();
    dataManager->getConf(inIds, outIds);
    EXPECT_EQ(inIds.size(), 0);
    EXPECT_EQ(outIds.size(), 0);
    // dataManager->setConf(1, 2, BROADCAST);
    dataManager->setFeeder("Feeder");
    dataManager->setConsumer("Consumer0");
    dataManager->setConsumer("Consumer1");
    // data manager thread
    // launch 1 thread per feeder
    std::thread t[4];
    t[0] = run_data_manager(dataManager);
    t[3] = run_feeder("Feeder", dataManager, tx_data, stopTest);
    // consumer thread
    for (int i = 1; i < 3; i++) {
        const std::string id = "Consumer" + std::to_string(i - 1);
        t[i]                 = run_consumer(id, dataManager, rx_data[i - 1], stopTest);
    }
    sleep(5);
    std::cout << "STOP" << std::endl;
    dataManager->stop();
    stopTest = true;
    for (int i = 0; i < 4; i++) {
        t[i].join();
    }

    // check tx vs rx data
    for (int i = 0; i < 2; i++) {
        for (auto it : rx_data[i]) {
            bool found = false;
            // check each tx is in rx
            for (auto ittx : tx_data) {
                if (it.compare(ittx) == 0)
                    found = true;
            }
            if (found) {
                std::cout << "Tx: " << it << " FOUND" << std::endl;
            } else {
                std::cout << "Tx: " << it << " KO" << std::endl;
            }
            EXPECT_EQ(found, true);
        }
    }
}
