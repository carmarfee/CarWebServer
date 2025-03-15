/**
 * @file sqlconnpool.cpp
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "sqlconnpool.h"

using namespace std;

SqlConnPool::SqlConnPool()
{
}

SqlConnPool::~SqlConnPool()
{
    close();
}

SqlConnPool *SqlConnPool::GetInstance()
{
    static SqlConnPool connPool;
    return &connPool;
}

void SqlConnPool::init(const char *host, int port,
                       const char *user, const char *pwd,
                       const char *dbName, int connSize)
{
    assert(connSize > 0);
    for (int i = 0; i < connSize; i++)
    {
        MYSQL *sql = nullptr;
        sql = mysql_init(sql);
        if (!sql)
        {
            LOG_ERROR("Mysql init error!");
            assert(sql);
        }
        sql = mysql_real_connect(sql, host, user, pwd, dbName, port, nullptr, 0);
        if (!sql)
        {
            LOG_ERROR("Mysql connect error!");
        }
        connQueue_.push(sql);
    }
    maxConn_ = connSize;
    sem_init(&sem_, 0, maxConn_);
}

MYSQL *SqlConnPool::GetConn()
{
    MYSQL *sql = nullptr;
    if (connQueue_.empty())
    {
        LOG_WARN("SqlConnPool busy!");
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

void SqlConnPool::DisConn(MYSQL *conn)
{
    assert(conn);
    lock_guard<mutex> locker(mtx_);
    connQueue_.push(conn);
    sem_post(&sem_);
}

int SqlConnPool::GetFreeConnCount()
{
    lock_guard<mutex> locker(mtx_);
    return connQueue_.size();
}

void SqlConnPool::close()
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
