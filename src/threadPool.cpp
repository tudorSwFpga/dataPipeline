#include "../include/threadPool.hpp"

void ThreadPool::start() {
    threads.resize(m_nproc);
    spdlog::debug("Starting N threads");
    for (uint32_t i = 0; i < m_nproc; i++) {
        threads.at(i) = std::thread(&ThreadPool::ThreadLoop, this, i);
    }
}

void ThreadPool::ThreadLoop(const int &id) {
    while (true) {
        std::pair<std::string, std::function<void()>> job;
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            mutex_condition.wait(lock, [this] { return !jobs.empty() || should_terminate; });
            if (should_terminate) {
                return;
            }
            job = jobs.front();
            jobs.pop();
        }
        spdlog::debug("ThreadLoop {} Running Job {}", id, job.first);
        job.second();
    }
}

void ThreadPool::QueueJob(const std::string &jobName, const std::function<void()> &job) {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        jobs.push(std::make_pair(jobName, job));
        // spdlog::debug("Queueing job {}", jobName);
    }
    mutex_condition.notify_one();
}

void ThreadPool::stop() {
    {
        std::unique_lock<std::mutex> lock(queue_mutex);
        should_terminate = true;
    }
    mutex_condition.notify_all();
    for (std::thread &active_thread : threads) {
        active_thread.join();
    }
    threads.clear();
}
