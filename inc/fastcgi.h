/**
 * @file fastcgi.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef FASTCGI_H
#define FASTCGI_H

#include <iostream>
#include <cstring>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
#include <unordered_map>

#include "buffer.h"
#include "global.h"
#include "log.h"

using namespace std;

class Fastcgi
{
public:
    Fastcgi()
    {
        iovCnt_ = 0;
        sockFd_ = -1;
    };
    ~Fastcgi()
    {
        if (sockFd_ > 0)
            close(sockFd_);
    };

    int GetFd() { return sockFd_; };

    void ConnectFcgiServer();
    void MakeFcgiRequest(string srcDir, string querystring, string method, string cgipath);
    int SendFcgiRequset();
    void ReadandParseFcgiResponse(Buffer &buff);

private:
    void BuildFcgiHeader_(Fcgiheader &header, uint8_t type, uint16_t requestId, uint16_t contentLength);
    void BuildFcgiBody_(FcgiBeginRequestBody &body);
    void BuildFcgiParams_(vector<uint8_t> &paramsbuffer);
    vector<uint8_t> encodeFastCgiParams(const std::unordered_map<std::string, std::string> &params);

private:
    Buffer writebuff_;
    Buffer readbuff_;

    int iovCnt_;
    struct iovec iov_[2];

    int sockFd_;
};

#endif // FASTCGI_H
