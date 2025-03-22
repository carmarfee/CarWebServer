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
    int connPoolNum, int threadNum, bool openlog, int loglevel, int logQueSize)
    : ip_(ip), port_(port), openlinger_(openlinger), timeoutMS_(timeoutMS), trigMode_(trigMode), listenFd_(-1), epoller_(new Epoller()), timer_(new heaptimer()), threadpool_(new threadpool(threadNum))
{
    Init();
    srcDir_ = getcwd(nullptr, 256);
    strncat(srcDir_, "/res/", 10);
    HttpConn::userCount = 0;
    HttpConn::srcDir = srcDir_;
    InitEventMode_(trigMode);
    if (!InitSocket_())
    {
        isClose_ = true;
    }
    isClose_ = false;
    if (openlog)
    {
        Log::GetInstance()->init(loglevel, "/log", ".log", logQueSize);
        if (isClose_)
        {
            LOG_ERROR("============== Server Init Error! ==============");
        }
        else
        {
            LOG_INFO("============== Server Init Success! ==============");
            LOG_INFO("          welcome to carmarfee's server           ");
            LOG_INFO("                                     / _|         ");
            LOG_INFO("  ___ __ _ _ __ _ __ ___   __ _ _ __| |_ ___  ___ ");
            LOG_INFO(" / __/ _` | '__| '_ ` _ \\ / _` | '__|  _/ _ \\/ _ \\");
            LOG_INFO("| (_| (_| | |  | | | | | | (_| | |  | ||  __/  __/");
            LOG_INFO(" \\___\\__,_|_|  |_| |_| |_|\\__,_|_|  |_| \\___|\\___|");
            LOG_INFO("--------------------------------------------------");
            LOG_INFO("                 the server config                ")
            LOG_INFO("--------------------------------------------------");
            LOG_INFO("IP: %s", ip_);
            LOG_INFO("Port: %d", port_);
            LOG_INFO("Listen Mode: %s", (listenEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("Connect Mode: %s", (connEvent_ & EPOLLET ? "ET" : "LT"));
            LOG_INFO("SysLogLevel: %d", loglevel);
            LOG_INFO("SqlConnPool num: %d", connPoolNum);
            LOG_INFO("ThreadPool num: %d", threadNum);
        }
    }
}

WebServer::~WebServer()
{
    close(listenFd_);
    isClose_ = true;
    free(srcDir_);
}

void WebServer::Start()
{
    int timeMS = -1; /* epoll wait timeout == -1 无事件将阻塞 */
    if (!isClose_)
    {
        LOG_INFO("============== Server Start Success!==============");
    }
    while (!isClose_)
    {
        if (timeoutMS_ > 0)
        {
            timeMS = timer_->GetNextTick(); // 获取下次事件的发生时间
        }
        int eventCnt = epoller_->Wait(timeMS); // 等待timeMS时间后epoll监听事件,并且此时恰好堆顶时间发生
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

void WebServer::InitEventMode_(int trigMode)
{
    listenEvent_ = EPOLLRDHUP;
    connEvent_ = EPOLLONESHOT | EPOLLRDHUP;
    switch (trigMode)
    {
    case 0:
        break;
    case 1:
        connEvent_ |= EPOLLET;
        break;
    case 2:
        listenEvent_ |= EPOLLET;
        break;
    case 3:
        listenEvent_ |= EPOLLET;
        connEvent_ |= EPOLLET;
        break;
    default:
        listenEvent_ = EPOLLET;
        connEvent_ = EPOLLET;
        break;
    }
}

bool WebServer::InitSocket_()
{
    int ret;
    struct sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    if ((port_ > 65535 || port_ < 1024))
    {
        LOG_ERROR("Port:%d error!", port_);
        return false;
    }
    addr.sin_family = AF_INET;
    inet_pton(AF_INET, ip_, &addr.sin_addr);
    addr.sin_port = htons(port_);
    listenFd_ = socket(AF_INET, SOCK_STREAM, 0);
    if (listenFd_ < 0)
    {
        LOG_ERROR("Create socket error!");
        return false;
    }
    // 设置延迟关闭套接字
    struct linger opt = {0, 0};
    if (openlinger_)
    {
        opt.l_onoff = 1;
        opt.l_linger = timeoutMS_ / 1000;
    }
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_LINGER, &opt, sizeof(opt));
    if (ret < 0)
    {
        LOG_ERROR("Init linger error!");
        close(listenFd_);
        return false;
    }
    // 设置地址复用
    int optval = 1;
    ret = setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval, sizeof(int));
    if (ret < 0)
    {
        LOG_ERROR("Set reuse address error!");
        close(listenFd_);
        return false;
    }
    ret = bind(listenFd_, (struct sockaddr *)&addr, sizeof(addr));
    if (ret < 0)
    {
        LOG_ERROR("Bind Port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    ret = listen(listenFd_, 6);
    if (ret < 0)
    {
        LOG_ERROR("Listen port:%d error!", port_);
        close(listenFd_);
        return false;
    }
    ret = epoller_->AddFd(listenFd_, listenEvent_ | EPOLLIN);
    if (ret == 0)
    {
        LOG_ERROR("Add listenfd[%d] to epoll error!", listenFd_);
        close(listenFd_);
        return false;
    }
    ret = SetFdNonblock_(listenFd_);
    if (ret < 0)
    {
        LOG_ERROR("Set Fdnonblock error!");
        close(listenFd_);
        return false;
    }
    return true;
}

int WebServer::SetFdNonblock_(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        return false;
    }
    flags |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, flags);
}

// 1.将连接到的用户添加到epoll中,用于监听是否有数据传递  2.将该连接事件(连接的socketFD)添加到timer定时器中,并定时为timeMS.如果没有数据写入则断开连接(短连接),否则有写事件发生就延迟该连接事件(fd)
void WebServer::AddClient_(int fd, sockaddr_in addr)
{
    assert(fd > 0);
    users_[fd].Init(fd, addr);
    if (timeoutMS_ > 0)
    {
        timer_->add(fd, timeoutMS_, std::bind(&WebServer::CloseConn_, this, &users_[fd]));
    }
    epoller_->AddFd(fd, EPOLLIN | connEvent_);
    SetFdNonblock_(fd);
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
            LOG_ERROR("Connect failed!");
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
    ret = client->Read(&readErrno);
    if (ret <= 0 && readErrno != EAGAIN)
    {
        CloseConn_(client);
        return;
    }
    OnProcess_(client);
}

void WebServer::OnProcess_(HttpConn *client)
{
    if (client->Process())
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
    ret = client->Write(&writeErrno);
    if (client->ToWriteBytes() == 0)
    {
        /* 传输完成 */
        if (client->IsKeepAlive())
        {
            OnProcess_(client);
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
