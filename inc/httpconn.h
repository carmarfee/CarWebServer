/**
 * @file httpconn.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef HTTPCONN_H
#define HTTPCONN_H

#include <sys/types.h>
#include <sys/uio.h>   // readv/writev
#include <arpa/inet.h> // sockaddr_in
#include <stdlib.h>    // atoi()
#include <errno.h>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <errno.h>
#include <cctype>

#include "log.h"
#include "buffer.h"
#include "util.h"
#include "global.h"
#include "fastcgi.h"

class HttpRequest
{
public:
    HttpRequest()
    {
        Init();
    }
    ~HttpRequest() = default;

    void Init()
    {
        requestmsg_.request_line = "";
        requestmsg_.request_header.clear();
        requestmsg_.request_body = "";

        state_ = REQUEST_LINE;
        method_ = path_ = version_ = "";
        header_kv_.clear();
        content_length_ = 0;
        query_string_ = "";
        cgi_ = false;
    };
    std::string GetMethod() const { return method_; };
    std::string GetPath() const { return path_; };
    std::string GetVersion() const { return version_; };

    bool ParseRequestMsg(Buffer &buff);

    // 心跳函数
    bool IsKeepAlive() const
    {
        if (header_kv_.count("Connection") == 1)
        {
            return header_kv_.find("Connection")->second == "keep-alive";
        }
        return false;
    };

private:
    bool ParseRequestLine_(const std::string &line);
    bool ParseHeader_(const std::string &headers);
    bool ParseBody_(const std::string &body);

public:
    RequestMsg requestmsg_; // 存放请求报文
    // 解析的结果
    std::string method_;                                     // 请求方法
    std::string path_;                                       // 请求路径
    std::string version_;                                    // 版本号
    std::unordered_map<std::string, std::string> header_kv_; // 请求报头中的键值对
    int content_length_;                                     // 正文长度
    std::string query_string_;                               // uri中携带的参数

    bool cgi_; // 是否启用cgi

private:
    PARSE_STATE state_;
};

class HttpResponse
{
public:
    HttpResponse()
    {
        code_ = -1;
        path_ = srcDir_ = "";
        isKeepAlive_ = false;
        file_content_ = nullptr;
        file_stat_ = {0};
    };
    ~HttpResponse()
    {
        Close();
    }
    void Init(const std::string &srcDir, std::string &path, bool isKeepAlive, int code)
    {
        responsemsg_.response_line = "";
        responsemsg_.response_header.clear();
        responsemsg_.response_body = "";

        code_ = code;
        path_ = path;
        srcDir_ = srcDir;
        isKeepAlive_ = isKeepAlive;
        file_content_ = nullptr;
        file_stat_ = {0};
    }

    void MakeResponseMsg(Buffer &buff);
    void Close();
    void ErrorContent(std::string message);

    char *GetContent(int cfrom = 0) const
    {
        return cfrom ? file_content_ : const_cast<char *>(responsemsg_.response_body.c_str()); // file_content只来自file,而请求体的内容可以是自己写的plain也可以是来自file
    }
    size_t GetContentLength(int lfrom) const
    {
        return lfrom ? file_stat_.st_size : responsemsg_.response_body.size();
    }

private:
    void AddResponseLine_(Buffer &buff);
    void AddResponseHeader_(Buffer &buff);
    void AddResponseBody_(Buffer &buff);

private:
    ResponseMsg responsemsg_; // 生成的响应报文
    // 所需数据
    char *file_content_;    // 文件内容
    struct stat file_stat_; // 文件状态

    std::string srcDir_; // 本地资源路径
    std::string path_;   // 请求资源路径

    bool isKeepAlive_; // 用于客户端的心跳函数检测

    int code_; // 状态码
};

class HttpConn
{
public:
    HttpConn()
    {
        fd_ = -1;
        addr_ = {0};
        isClose_ = true;
    };
    ~HttpConn()
    {
        Close();
    };

    void Init(int sockfd, const sockaddr_in &addr);
    void Close();
    bool Process();
    ssize_t Read(int *Errno);
    ssize_t Write(int *Errno);

    int GetFd() const { return fd_; };
    int GetCgiFd() { return cgiserver_.GetFd(); };
    int IsCgi() { return request_.cgi_; };
    int GetPort() const { return addr_.sin_port; };
    const char *GetIP() const { return inet_ntoa(addr_.sin_addr); }
    sockaddr_in GetAddr() const { return addr_; };

    int ToWriteBytes()
    {
        return iov_[0].iov_len + iov_[1].iov_len;
    }

    bool IsKeepAlive() const
    {
        return request_.IsKeepAlive();
    }

public:
    static const char *srcDir;
    static std::atomic<int> userCount;

private:
    int fd_;

    Fastcgi cgiserver_;

    struct sockaddr_in addr_;
    bool isClose_;

    Buffer readBuff_;  // 读缓冲区
    Buffer writeBuff_; // 写缓冲区

    int iovCnt_;
    struct iovec iov_[2];

    HttpRequest request_;
    HttpResponse response_;

    Fastcgi fcgi_;
};

#endif // HTTPCONN_H