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
class blockqueue {
public:
    explicit blockqueue(size_t maxcapacity = 1000);
    ~blockqueue();
    bool push(const T &item);
    bool pop(T &item, int timeout = 0);
    void close();

private:
    // members
    std::deque<T> queue_;
    size_t capacity_;
    std::mutex mutex_;
    std::condition_variable condconsumer_;
    std::condition_variable condproducer_;
    bool isclosed_;
};
