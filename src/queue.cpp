#include <iostream>
#include <algorithm>
#include <utility>

template<class T>
void Q<T>::push(const T &data) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_back(data);
    cv.notify_all();
}

template<class T>
void Q<T>::push(const T &&data) {
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_back(data);
    cv.notify_all();
}

template<class T>
void Q<T>::pop(T &data, bool blocking) {
    std::unique_lock<std::mutex> lock(mtx);
    if (blocking)
        cv.wait(lock);
    data = queue.front();
    queue.pop_front();
    cv.notify_all();
}

template<class T>
void Q<T>::pop(bool blocking) {
    std::unique_lock<std::mutex> lock(mtx);
    if (blocking)
        cv.wait(lock);
    queue.pop_front();
    cv.notify_all();
}

template<class T>
void Q<T>::copy(Q<T> &other) {
    std::unique_lock<std::mutex> lock_src(mtx);
    std::unique_lock<std::mutex> lock_dst(other.mtx);
    std::copy(queue.begin(), queue.end(), std::back_inserter(other.queue));
    other.cv.notify_all();
}

// copy one queue to several queues, with a list saying which ones have to be copied
template<class T>
void Q<T>::copy(std::vector<Q<T>> &others, std::list<int> enableCopy) {
    std::unique_lock<std::mutex> lock_src(mtx);
    for (auto id : enableCopy) {
        std::unique_lock<std::mutex> lock_dst(others[id].mtx);
        std::copy(queue.begin(), queue.end(), std::back_inserter(others[id].queue));
        others[id].cv.notify_all();
    }
    queue.clear();
}

template<class T>
void Q<T>::move(std::vector<T> &other) {
    std::unique_lock<std::mutex> lock_src(mtx);
    // TODO: moving works with strings but will not work with some types such as integers
    other.insert(other.end(), std::make_move_iterator(queue.begin()), std::make_move_iterator(queue.end()));
    queue.clear();
}

template<class T>
void Q<T>::stop() {
    cv.notify_all();
}

template<class T>
bool Q<T>::empty() {
    std::unique_lock<std::mutex> lock(mtx);
    return queue.empty();
}

template<class T>
T Q<T>::front() {
    return queue.front();
}

template<class T>
void Q<T>::pause() {
    mtx.lock();
}

template<class T>
void Q<T>::restart() {
    queue.clear();
    mtx.unlock();
}

template class Q<std::string>;
