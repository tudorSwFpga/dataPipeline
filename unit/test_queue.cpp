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
#include "../include/queue.hpp"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

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

std::thread run_consumer(std::shared_ptr<Q<std::string>> q, std::vector<std::string> &rx_data,
                         std::atomic<bool> &stop) {
    std::thread consumerThread([&q, &stop, &rx_data]() {
        spdlog::debug("consumer thread running");
        std::string msg;
        while (!stop) {
            q->pop(msg);
            rx_data.push_back(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    return consumerThread;
}

std::thread run_feeder(std::shared_ptr<Q<std::string>> q, std::vector<std::string> &tx_data, std::atomic<bool> &stop) {
    std::thread feederThread([&q, &stop, &tx_data]() {
        spdlog::debug("run feeder thread");
        while (!stop) {
            std::string msg = gen_random(16);
            tx_data.push_back(msg);
            q->push(msg);
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        }
    });

    return feederThread;
}

TEST(DataManager, FeedersAndConsumers) {
    spdlog::set_level(spdlog::level::debug);
    std::shared_ptr<Q<std::string>> q = std::make_shared<Q<std::string>>();
    std::vector<std::string> tx_data, rx_data;
    std::atomic<bool> stopTest(false);
    std::thread feeder, consumer;
    feeder   = run_feeder(q, tx_data, stopTest);
    consumer = run_consumer(q, rx_data, stopTest);
    sleep(5);
    std::cout << "STOP" << std::endl;
    stopTest = true;
    feeder.join();
    consumer.join();

    // check tx vs rx data
    for (auto it : tx_data) {
        bool found = false;
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
