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
#include "../log/log.h"
#include "../timer/heaptimer.h"
#include "../pool/sqlconnpool.h"
#include "../pool/threadpool.h"
#include "../pool/sqlconnRAII.h"
#include "../http/httpconn.h"

class WebServer
{
public:
    WebServer(const char *ip, int port, bool openlinger, int timeoutlingersec, int trigMode);
    ~WebServer();

    void Start();

private:
    void InitEventMode_();
    bool InitSocket_();
    void AddClient_(int fd, sockaddr_in addr);

    const char *ip_;
    int port_;

    bool openlinger;
    int timeoutlingersec;

    int trigMode_;

    int listenFd_;

    uint32_t listenEvent_;

    std::unique_ptr<Epoller> epoller_;
};

#endif // __WEBSERVER_H__