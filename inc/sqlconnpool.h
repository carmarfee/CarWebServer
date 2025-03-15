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

#include "log.h"

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
    SqlConnPool();
    ~SqlConnPool();

private:
    // members
    int maxConn_;
    std::queue<MYSQL *> connQueue_;
    std::mutex mtx_;
    sem_t sem_;
};

#endif // SQLCONNPOOL_H