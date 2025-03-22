/**
 * @file sqlconnpool.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef SQLCONNPOOL_H
#define SQLCONNPOOL_H

#include <mysql/mysql.h>
#include <string>
#include <queue>
#include <mutex>
#include <semaphore.h>
#include <thread>
#include <assert.h>

using namespace std;

class SqlConnPool
{
public:
    static SqlConnPool *GetInstance();
    void init(const char *host, int port,
              const char *user, const char *pwd,
              const char *dbName, int connSize);
    MYSQL *GetConn();
    void DisConn(MYSQL *conn);
    int GetFreeConnCount();
    void close();

private:
    SqlConnPool() = default;
    ~SqlConnPool();

private:
    // members
    int maxConn_;
    std::queue<MYSQL *> connQueue_;
    std::mutex mtx_;
    sem_t sem_;
};

inline SqlConnPool::~SqlConnPool()
{
    close();
}

SqlConnPool *SqlConnPool::GetInstance()
{
    static SqlConnPool connPool;
    return &connPool;
}

inline void SqlConnPool::init(const char *host, int port,
                              const char *user, const char *pwd,
                              const char *dbName, int connSize)
{
    for (int i = 0; i < connSize; i++)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql)
        {
            assert(sql);
        }
        connQueue_.push(sql);
    }
    maxConn_ = connSize;
    sem_init(&sem_, 0, maxConn_);
}

inline MYSQL *SqlConnPool::GetConn()
{
    MYSQL *sql = nullptr;
    if (connQueue_.empty())
    {
        return nullptr;
    }
    sem_wait(&sem_);
    {
        lock_guard<mutex> locker(mtx_);
        sql = connQueue_.front();
        connQueue_.pop();
    }
    return sql;
}

inline void SqlConnPool::DisConn(MYSQL *conn)
{
    assert(conn);
    lock_guard<mutex> locker(mtx_);
    connQueue_.push(conn);
    sem_post(&sem_);
}

inline int SqlConnPool::GetFreeConnCount()
{
    lock_guard<mutex> locker(mtx_);
    return connQueue_.size();
}

inline void SqlConnPool::close()
{
    lock_guard<mutex> locker(mtx_);
    while (!connQueue_.empty())
    {
        auto item = connQueue_.front();
        connQueue_.pop();
        mysql_close(item);
    }
    mysql_library_end();
}

#endif // SQLCONNPOOL_H