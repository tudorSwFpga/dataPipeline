#include <functional>
#include <mutex>
#include <vector>
#include <queue>
#include <condition_variable>
#include <thread>
#include <utility>
#include "spdlog/spdlog.h"
#pragma once

class ThreadPool {
public:
    ThreadPool(const uint32_t &nproc) : m_nproc(nproc) {
        spdlog::debug("ThreadPool ctor with {} threads", nproc);
    };
    ~ThreadPool() {
        spdlog::debug("ThreadPool dtor");
    };
    void start();
    void QueueJob(const std::string &jobName, const std::function<void()> &job);
    void stop();

private:
    void ThreadLoop(const int &id);
    uint32_t m_nproc;
    bool should_terminate = false;           // Tells threads to stop looking for jobs
    std::mutex queue_mutex;                  // Prevents data races to the job queue
    std::condition_variable mutex_condition; // Allows threads to wait on new jobs or termination
    std::vector<std::thread> threads;
    std::queue<std::pair<std::string, std::function<void()>>> jobs;
};
