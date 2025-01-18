#pragma once
#include <mutex>
#include <deque>
#include <vector>
#include <condition_variable>
#include <list>
#include <iostream>
#include "spdlog/spdlog.h"

template<class T>
class Q {
public:
    Q() {
        spdlog::debug("Q ctor");
    };

    // Delete the copy constructor and copy assignment operator
    Q(const Q &)            = delete;
    Q &operator=(const Q &) = delete;

    // Move constructor
    Q(Q &&other) noexcept {
        std::lock_guard<std::mutex> lock(other.mtx); // Ensure thread-safety during move
        queue = std::move(other.queue);
    }

    // Define move assignment operator
    Q &operator=(Q &&other) noexcept {
        if (this != &other) {
            std::lock_guard<std::mutex> lock(other.mtx); // Ensure thread-safety during move
            queue = std::move(other.queue);
        }
        return *this;
    }

    ~Q() {
        spdlog::debug("Q dtor");
    };

    // copy data to another Q
    void copy(Q<T> &other);
    // copy data to several Q, enable signal saying to which
    void copy(std::vector<Q<T>> &others, std::list<int> enableCopy);
    // move data to another Q
    void move(std::vector<T> &other);
    // push data into Q
    void push(const T &data);
    void push(const T &&data);

    void pop(T &data, bool blocking);
    void pop(bool blocking);
    void stop();
    void pause();
    void restart();
    T front();
    bool empty();

    std::deque<T> queue;
    std::mutex mtx;             // Mutex to protect the queue
    std::condition_variable cv; // Condition variable to notify the consumer
};

#include "../src/queue.cpp"
