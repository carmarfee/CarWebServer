/**
 * @file webserver.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __WEBSERVER_H__
#define __WEBSERVER_H__

#include <unordered_map>
#include <fcntl.h>  // fcntl()
#include <unistd.h> // close()
#include <assert.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "epoller.h"
#include "log.h"
#include "heaptimer.h"
#include "threadpool.h"
#include "sqlconnpool.h"
#include "sqlconn.h"
#include "httpconn.h"

class WebServer
{
public:
    WebServer(
        const char *ip, int port, bool openlinger, int timeoutMS, int trigMode,
        int sqlport, const char *sqluser, const char *sqlpwd, const char *dbname,
        int connPoolNum, int threadNum, bool openlog, int loglevel, int logQueSize);
    ~WebServer();

    void Start();
    void Init()
    {
        isClose_ = true;
        srcDir_ = nullptr;

        listenEvent_ = 0;
        connEvent_ = 0;
    };

private:
    bool InitSocket_();
    void InitEventMode_(int trigMode);
    void AddClient_(int fd, sockaddr_in addr);
    void DealRead_(HttpConn *client);
    void DealWrite_(HttpConn *client);
    void DealListen_();
    void CloseConn_(HttpConn *client);
    void ExtentTime_(HttpConn *client);
    void SendError_(int fd, const char *info);

    void OnRead_(HttpConn *client);
    void OnWrite_(HttpConn *client);
    void OnProcess_(HttpConn *client);
    static int SetFdNonblock_(int fd);

private:
    static const int MAX_FD = 65536;

    const char *ip_;
    int port_;
    bool openlinger_;
    int timeoutMS_; /* 毫秒MS */
    int trigMode_;
    bool isClose_;
    int listenFd_;
    char *srcDir_;

    uint32_t listenEvent_;
    uint32_t connEvent_;

    std::unique_ptr<heaptimer> timer_;
    std::unique_ptr<threadpool> threadpool_;
    std::unique_ptr<Epoller> epoller_;
    std::unordered_map<int, HttpConn> users_;
};

#endif // __WEBSERVER_H__