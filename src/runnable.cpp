#include "../include/runnable.hpp"

// TODO: convert this to bool or int to return something
bool Runnable::run() {
    m_isRunning = true;
    return true;
}

bool Runnable::stop() {
    m_isRunning = false;
    return true;
}
