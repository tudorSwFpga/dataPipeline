#include "../include/runnable.hpp"

// TODO: convert this to bool or int to return something
void Runnable::run() {
    m_isRunning = true;
}

void Runnable::stop() {
    m_isRunning = false;
}
