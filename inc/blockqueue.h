/**
 * @file blockqueue.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef BLOCKQUEUE_H
#define BLOCKQUEUE_H

#include <mutex>
#include <deque>
#include <condition_variable>
#include <sys/time.h>

template <typename T>
class blockqueue
{
public:
    explicit blockqueue(size_t maxcapacity = 1000);
    ~blockqueue();
    bool push(const T &item);
    bool pop(T &item);
    bool pop(T &item, int timeout);
    void close();
    void flush();
    bool empty();
    bool full();

private:
    // members
    std::deque<T> queue_;
    size_t capacity_;
    std::mutex mutex_;
    std::condition_variable condconsumer_;
    std::condition_variable condproducer_;
    bool isclosed_;
};

template <class T>
blockqueue<T>::blockqueue(size_t maxcapacity) : capacity_(maxcapacity), isclosed_(false) {}

template <class T>
blockqueue<T>::~blockqueue()
{
    close();
}

template <class T>
void blockqueue<T>::close()
{
    {
        std::lock_guard<std::mutex> lock(mutex_);
        queue_.clear();
        isclosed_ = true;
    }
    condproducer_.notify_all();
    condconsumer_.notify_all();
}

template <class T>
bool blockqueue<T>::push(const T &item)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.size() >= capacity_)
    {
        condproducer_.wait(lock);
    }
    if (isclosed_)
    {
        return false;
    }
    queue_.push_back(item);
    condconsumer_.notify_one();
    return true;
}

template <class T>
bool blockqueue<T>::pop(T &item)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty())
    {
        condconsumer_.wait(lock);
        if (isclosed_)
        {
            return false;
        }
    }
    item = queue_.front();
    queue_.pop_front();
    condproducer_.notify_one();
    return true;
}

template <class T>
bool blockqueue<T>::pop(T &item, int timeout)
{
    std::unique_lock<std::mutex> lock(mutex_);
    while (queue_.empty())
    {
        if (condconsumer_.wait_for(lock, std::chrono::seconds(timeout)) == std::cv_status::timeout)
        {
            return false;
        }
        if (isclosed_)
        {
            return false;
        }
    }
    item = queue_.front();
    queue_.pop_front();
    condproducer_.notify_one();
    return true;
}

template <class T>
void blockqueue<T>::flush()
{
    condconsumer_.notify_one();
}

template <class T>
bool blockqueue<T>::empty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

template <class T>
bool blockqueue<T>::full()
{
    std::lock_guard<std::mutex> locker(mutex_);
    return queue_.size() >= capacity_;
}

#endif // BLOCKQUEUE_H