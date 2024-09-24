#include <iostream>

template<class T>
void Q<T>::push(const T &data) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push(data);
    cv.notify_all();
}

template<class T>
void Q<T>::pop(T &data) {
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock);
    data = queue.front();
    queue.pop();
    cv.notify_all();
}

template<class T>
void Q<T>::stop() {
    cv.notify_all();
}

template class Q<std::string>;
