#pragma once
#include <atomic>

class Runnable {
public:
    Runnable() : m_isRunning(false) {}
    ~Runnable() {}

    virtual void run();
    virtual void stop();

protected:
    std::atomic<bool> m_isRunning;
};
