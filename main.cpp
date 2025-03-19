/**
 * @file main.cpp
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-15
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <unistd.h>
#include "inc/webserver.h"

int main()
{
    WebServer server(
        "127.0.0.1", 1316, false, 60000, 3, /* 端口 ET模式 timeoutMs 优雅退出  */
        3306, "root", "root", "mydb",       /* Mysql配置 */
        12, 6, true, 1, 1024);              /* 连接池数量 线程池数量 日志开关 日志等级 日志异步队列容量 */
    server.Start();
}