#pragma once
#include <atomic>

class Runnable {
public:
    Runnable() : m_isRunning(false) {}
    ~Runnable() {}

    virtual bool run();
    virtual bool stop();

protected:
    std::atomic<bool> m_isRunning;
};
