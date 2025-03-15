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

#include "../../inc/webserver.h"
using namespace std;

WebServer::WebServer(
    const char *ip, int port, bool openlinger, int timeoutMS, int trigMode,
    int sqlport, const char *sqluser, const char *sqlpwd, const char *dbname,
    int connPoolNum, int threadNum, bool openlog, int loglevel, int logQueSize)
    : ip_(ip), port_(port), openlinger_(openlinger), timeoutMS_(timeoutMS), trigMode_(trigMode), listenFd_(-1), epoller_(new Epoller())
{
    srcDir_ = getcwd(nullptr, 256);
    assert(srcDir_);
    strncat(srcDir_, "/res/", 10);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    SqlConnPool::GetInstance()->init("localhost", sqlport, sqluser, sqlpwd, dbname, connPoolNum);
    InitEventMode_();
    if (!InitSocket_())
    {
        isClose_ = true;
    }
    if (openlog)
    {
        Log::GetInstance()->init(loglevel, "./log", ".log", logQueSize);
        if (isClose_)
        {
            LOG_ERROR("========== Server init error!==========");
        }
        else
        {
            LOG_INFO("========== Server init ==========");
            LOG_INFO("Port:%d, OpenLinger: %s", port_, openlinger_ ? "true" : "false");
            LOG_INFO("Listen Mode: %s, OpenConn Mode: %s",
                     (listenEvent_ & EPOLLET ? "ET" : "LT"),
                     (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("LogSys level: %d", loglevel);
            LOG_INFO("srcDir: %s", HttpConn::srcDir);
            LOG_INFO("SqlConnPool num: %d, ThreadPool num: %d", connPoolNum, threadNum);
        }
    }
}

WebServer::~WebServer()
{
    cout << "WebServer destructed" << endl;
}

void WebServer::Start()
{
    int timeMS = -1; /* epoll wait timeout == -1 无事件将阻塞 */
    if (!isClose_)
    {
        LOG_INFO("========== Server start ==========");
    }
    while (!isClose_)
    {
        if (timeoutMS_ > 0)
        {
            timeMS = timer_->GetNextTick();
        }
        int eventCnt = epoller_->Wait(timeMS);
        for (int i = 0; i < eventCnt; i++)
        {
            /* 处理事件 */
            int fd = epoller_->GetEventFd(i);
            uint32_t events = epoller_->GetEvents(i);
            if (fd == listenFd_)
            {
                DealListen_();
            }
            else if (events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR))
            {
                assert(users_.count(fd) > 0);
                CloseConn_(&users_[fd]);
            }
            else if (events & EPOLLIN)
            {
                assert(users_.count(fd) > 0);
                DealRead_(&users_[fd]);
            }
            else if (events & EPOLLOUT)
            {
                assert(users_.count(fd) > 0);
                DealWrite_(&users_[fd]);
            }
            else
            {
                LOG_ERROR("Unexpected event");
            }
        }
    }
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
    if (openlinger_)
    {
        optlinger.l_onoff = 1;
        optlinger.l_linger = timeoutMS_ / 1000;
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

void WebServer::AddClient_(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    users_[fd].init(fd, addr);
    if (timeoutMS_ > 0)
    {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock(fd);
    LOG_INFO("Client[%d] in!", users_[fd].GetFd());
}

void WebServer::SendError_(int fd, const char *info)
{
    assert(fd > 0);
    int ret = send(fd, info, strlen(info), 0);
    if (ret < 0)
    {
        LOG_WARN("send error to client[%d] error!", fd);
    }
    close(fd);
}

void WebServer::CloseConn_(HttpConn *client)
{
    assert(client);
    LOG_INFO("Client[%d] quit!", client->GetFd());
    epoller_->DelFd(client->GetFd());
    client->Close();
}

void WebServer::DealListen_()
{
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    do
    {
        int fd = accept(listenFd_, (struct sockaddr *)&addr, &len);
        if (fd <= 0)
        {
            return;
        }
        else if (HttpConn::userCount >= MAX_FD)
        {
            SendError_(fd, "Server busy!");
            LOG_WARN("Clients is full!");
            return;
        }
        AddClient_(fd, addr);
    } while (listenEvent_ & EPOLLET);
}

void WebServer::DealRead_(HttpConn *client)
{
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnRead_, this, client));
}

void WebServer::DealWrite_(HttpConn *client)
{
    assert(client);
    ExtentTime_(client);
    threadpool_->AddTask(std::bind(&WebServer::OnWrite_, this, client));
}

void WebServer::ExtentTime_(HttpConn *client)
{
    assert(client);
    if (timeoutMS_ > 0)
    {
        timer_->adjust(client->GetFd(), timeoutMS_);
    }
}

void WebServer::OnRead_(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int readErrno = 0;
    ret = client->read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN)
    {
        CloseConn_(client);
        return;
    }
    OnProcess(client);
}

void WebServer::OnProcess(HttpConn *client)
{
    if (client->process())
    {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
    }
    else
    {
        epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLIN);
    }
}

void WebServer::OnWrite_(HttpConn *client)
{
    assert(client);
    int ret = -1;
    int writeErrno = 0;
    ret = client->write(&writeErrno);
    if (client->ToWriteBytes() == 0)
    {
        /* 传输完成 */
        if (client->IsKeepAlive())
        {
            OnProcess(client);
            return;
        }
    }
    else if (ret < 0)
    {
        if (writeErrno == EAGAIN)
        {
            /* 继续传输 */
            epoller_->ModFd(client->GetFd(), connEvent_ | EPOLLOUT);
            return;
        }
    }
    CloseConn_(client);
}
