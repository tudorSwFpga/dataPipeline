#pragma once
#include <mutex>
#include <queue>
#include <condition_variable>

template<class T>
class Q {
public:
    Q(){};
    ~Q(){};
    void push(const T &);
    void pop(T &data);
    void stop();

private:
    std::queue<T> queue;        // Dedicated queue for each consumer
    std::mutex mtx;             // Mutex to protect the queue
    std::condition_variable cv; // Condition variable to notify the consumer
};

#include "../src/queue.cpp"
