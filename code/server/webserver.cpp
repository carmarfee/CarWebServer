/**
 * @file webserver.cpp
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-03
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "webserver.h"

using namespace std;

WebServer::WebServer(
    const char *ip, int port, bool openlinger, int timeoutlingersec, int trigMode)
    : ip_(ip), port_(port), openlinger(openlinger), timeoutlingersec(timeoutlingersec), trigMode_(trigMode_), listenFd_(-1), epoller_(new Epoller())
{
    cout << "WebServer constructed" << endl;
}

WebServer::~WebServer()
{
    cout << "WebServer destructed" << endl;
}

void WebServer::Start()
{
    
}

void WebServer::InitEventMode_()
{
    listenEvent_ = EPOLLRDHUP;
    if (trigMode_ == 0)
    {
        cout << "InitEventMode_ is LT" << endl;
    }
    else if (trigMode_ == 1)
    {
        listenEvent_ |= EPOLLET;
        cout << "InitEventMode_ is ET" << endl;
    }
}

bool WebServer::InitSocket_()
{
    int ret;
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    if ((port_ > 65535 || port_ < 1024))
    {
        std::cout << "Port: " << port_ << " is invalid!" << endl;
        return false;
    }
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_, &addr.sin_addr);
    addr.sin_port = htons(port_);
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0)
    {
        std::cout << "Create socket failed!" << endl;
        return false;
    }
    // 设置延迟关闭套接字
    struct linger optlinger = {0, 0};
    if (openlinger)
    {
        optlinger.l_onoff = 1;
        optlinger.l_linger = timeoutlingersec;
    }
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &optlinger, sizeof(optlinger));
    if (ret < 0)
    {
        std::cout << "Set linger failed!" << endl;
        close(listenFd_);
        return false;
    }
    // 设置端口复用
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if (ret < 0)
    {
        std::cout << "Set reuseaddr failed!" << endl;
        close(listenFd_);
        return false;
    }
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        std::cout << "Bind port: " << port_ << " failed!" << endl;
        close(listenFd_);
        return false;
    }
    ret = listen(listenFd_, 6);
    if (ret < 0)
    {
        std::cout << "Listen port: " << port_ << " failed!" << endl;
        close(listenFd_);
        return false;
    }
    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret == 0)
    {
        std::cout << "Add listen socket to epoll successfully!" << endl;
        return true;
    }
    ret = SetFdNonblock(listenFd_);
    if (ret < 0)
    {
        std::cout << "Set nonblock failed!" << endl;
        close(listenFd_);
        return false;
    }
    return true;
}

int WebServer::SetFdNonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        return false;
    }
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}