#include <memory>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <time.h>
#include <memory>
#include <thread>
#include <atomic>

// class under test
#include "../include/threadPool.hpp"
#include "gtest/gtest.h"
#include "spdlog/spdlog.h"

class test {
public:
    // test(){};
    //~test(){}

    void run() {
        std::string txt = gen_random(128);
        std::lock_guard<std::mutex> lock(mtx);
        vals.push_back(txt);
    }

    int getStringCnt() {
        return vals.size();
    }

private:
    std::mutex mtx;
    std::vector<std::string> vals;

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
};

TEST(TP, 4Threads) {
    spdlog::set_level(spdlog::level::debug);
    const int threadCount          = 4;
    std::unique_ptr<ThreadPool> tp = std::make_unique<ThreadPool>(threadCount);
    tp->start();
    test t;
    for (int i = 0; i < 100; i++) {
        const std::string jn = "GenerateString" + std::to_string(i);
        tp->QueueJob(jn, std::bind(&test::run, &t));
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(5000));
    tp->stop();
    EXPECT_EQ(t.getStringCnt(), 100);
}
