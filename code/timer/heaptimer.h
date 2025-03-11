/**
 * @file heaptimer.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __HEAPTIMER_H__
#define __HEAPTIMER_H__

#include <queue>
#include <unordered_map>
#include <time.h>
#include <algorithm>
#include <arpa/inet.h>
#include <functional>
#include <assert.h>
#include <chrono>
#include "../log/log.h"

typedef std::function<void()> TimeoutCallBack;
typedef std::chrono::high_resolution_clock Clock;
typedef std::chrono::milliseconds ms;
typedef Clock::time_point timepoint;

class timer
{
public:
    int id;
    timepoint expires;
    TimeoutCallBack cb;
    bool operator<(const timer &t)
    {
        return expires < t.expires;
    }
};

class heaptimer
{
public:
    heaptimer() { heap_.reserve(64); };
    ~heaptimer();

    void add(int id, int timeout, const TimeoutCallBack &cb);
    void pop();
    void tick();

private:
    // members
    void del_(size_t index);
    void siftup_(size_t i);
    void siftdown_(size_t index, size_t n);

    std::vector<timer> heap_;
    std::unordered_map<int, size_t> pos_;
};

#endif // __HEAPTIMER_H__