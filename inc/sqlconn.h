/**
 * @file sqlconn.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef SQLCONN_H
#define SQLCONN_H
#include "sqlconnpool.h"

class SqlConn
{
public:
    SqlConn(MYSQL **sql, SqlConnPool *connpool)
    {
        assert(connpool);
        *sql = connpool->GetConn();
        sql_ = *sql;
        connpool_ = connpool;
    }

    ~SqlConn()
    {
        if (sql_)
        {
            connpool_->DisConn(sql_);
        }
    }

private:
    MYSQL *sql_;
    SqlConnPool *connpool_;
};

#endif // SQLCONN_H