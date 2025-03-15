/**
 * @file threadpool.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef THREADPOOL_H
#define THREADPOOL_H

#include <mutex>
#include <condition_variable>
#include <queue>
#include <thread>
#include <functional>

template <class F>
class threadpool
{
public:
    explicit threadpool(size_t threadcount = 8);
    ~threadpool();
    void AddTask(F &&task);

private:
    // members
    struct Pool
    {
        std::mutex mtx;
        std::condition_variable cv;
        bool isclose;
        std::queue<F> tasks;
    };
    std::shared_ptr<Pool> pool_;
};

template <class F>
threadpool<F>::threadpool(size_t threadcount) : pool_(std::make_shared<Pool>())
{
    for (size_t i = 0; i < threadcount; i++)
    {
        std::thread([pool = pool_]
                    {
            std::unique_lock<std::mutex> locker(pool->mtx);
            while (true)
            {
                if (!pool->tasks.empty())
                {
                    auto task = std::move(pool->tasks.front());
                    pool->tasks.pop();
                    locker.unlock();
                    task();
                    locker.lock();
                }
                else if (pool->isclose)
                {
                    break;
                }
                else
                {
                    pool->cv.wait(locker);
                }
            } })
            .detach();
    }
}

template <class F>
threadpool<F>::~threadpool()
{
    if (static_cast<bool> pool_)
    {
        {
            std::lock_guard<std::mutex> locker(pool_->mtx);
            pool_->isclose = true;
        }
        pool_->cv.notify_all();
    }
}

template <class F>
void threadpool<F>::AddTask(F &&task)
{
    {
        std::lock_guard<std::mutex> locker(pool_->mtx);
        pool_->tasks.emplace(std::forward<F>(task));
    }
    pool_->cv.notify_one();
}

#endif // THREADPOOL_H