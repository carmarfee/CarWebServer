/**
 * @file fastcgi.c
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-19
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../../inc/fastcgi.h"

void Fastcgi::ConnectFcgiServer()
{
    sockFd_ = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(10000);
    inet_pton(AF_INET, "127.0.0.1", &serverAddr.sin_addr); // 连接php-fpm服务器
    if (connect(sockFd_, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) < 0)
    {
        LOG_ERROR("连接PHP-FPM失败");
    }
}

void Fastcgi::BuildFcgiHeader_(Fcgiheader &header, uint8_t type, uint16_t requestId, uint16_t contentLength)
{
    header.version = FCGI_VERSION_1;
    header.type = type;
    header.requestId = htons(requestId);
    header.contentLength = htons(contentLength);
    header.paddingLength = 0;
    header.reserved = 0;
    writebuff_.Append(&header, sizeof(header));
}

void Fastcgi::BuildFcgiBody_(FcgiBeginRequestBody &body)
{
    body.role = htons(FCGI_RESPONDER);
    body.flags = 0;
    memset(body.reserved, 0, sizeof(body.reserved));
    writebuff_.Append(&body, sizeof(body));
}

void Fastcgi::BuildFcgiParams_(vector<uint8_t> &paramsbuffer)
{
    writebuff_.Append(paramsbuffer.data(), paramsbuffer.size());
}

void Fastcgi::MakeFcgiRequest(string srcDir, string querystring, string method, string cgipath)
{
    Fcgiheader header;
    FcgiBeginRequestBody body;
    BuildFcgiHeader_(header, FCGI_BEGIN_REQUEST, 1, sizeof(header));
    BuildFcgiBody_(body);

    size_t pos = cgipath.find('/');
    string cginame = cgipath.substr(pos + 1);

    vector<uint8_t> paramsbuffer = encodeFastCgiParams({{"SCRIPT_FILENAME", srcDir + "../backend" + cgipath},
                                                        {"QUERY_STRING", querystring},
                                                        {"REQUEST_METHOD", method},
                                                        {"SCRIPT_NAME", cginame},
                                                        {"REQUEST_URI", "/backend" + cgipath + "?" + querystring},
                                                        {"DOCUMENT_ROOT", "/home/carmarfee/C++pro/CarWebServer/res"},
                                                        {"SERVER_PROTOCOL", "HTTP/1.1"},
                                                        {"GATEWAY_INTERFACE", "CGI/1.1"},
                                                        {"SERVER_SOFTWARE", "MyFastCGI/1.0"},
                                                        {"REMOTE_ADDR", "127.0.0.1"},
                                                        {"REMOTE_PORT", "56789"},
                                                        {"SERVER_ADDR", "127.0.0.1"},
                                                        {"SERVER_PORT", "10000"},
                                                        {"CONTENT_LENGTH", "0"},
                                                        {"CONTENT_TYPE", ""}});
    BuildFcgiHeader_(header, FCGI_PARAMS, 1, paramsbuffer.size());
    BuildFcgiParams_(paramsbuffer);

    BuildFcgiHeader_(header, FCGI_PARAMS, 1, 0);
}

int Fastcgi::SendFcgiRequset()
{
    iov_[0].iov_base = const_cast<char *>(writebuff_.ReadPosAddr());
    iov_[0].iov_len = writebuff_.ReadableBytes();

    ssize_t len = -1;
    while (true)
    {
        len = writev(sockFd_, iov_, 1);
        iov_[0].iov_base = (uint8_t *)iov_[0].iov_base + len;
        iov_[0].iov_len -= len;
        writebuff_.Retrieve(len);
        if (iov_[0].iov_len == 0)
        {
            break;
        }
    }
    return len;
}

void Fastcgi::ReadandParseFcgiResponse(Buffer &buff)
{
    ssize_t len = -1;
    int *readerror = 0;
    Fcgiheader *beginheader;
    bool isgetheader = false;
    int contentlength = 0;
    while (true)
    {
        len = readbuff_.ReadFd(sockFd_, readerror);
        if (len <= 0)
            break;
        if (readbuff_.ReadableBytes() >= 8 && !isgetheader)
        {
            isgetheader = true;
            beginheader = (Fcgiheader *)readbuff_.ReadPosAddr();
            contentlength = ntohs(beginheader->contentLength);
        }
        if (contentlength != 0 && readbuff_.ReadableBytes() >= 8 + contentlength)
            break;
    }
    buff.Append(readbuff_.ReadPosAddr() + sizeof(beginheader), contentlength);
    close(sockFd_);
}

std::vector<uint8_t> Fastcgi::encodeFastCgiParams(const std::unordered_map<std::string, std::string> &params)
{
    std::vector<uint8_t> buffer;

    for (const auto &param : params)
    {
        std::string key = param.first;
        std::string value = param.second;

        size_t keyLen = key.size();
        size_t valueLen = value.size();

        // 处理 key 长度
        if (keyLen < 128)
        {
            buffer.push_back(static_cast<uint8_t>(keyLen));
        }
        else
        {
            buffer.push_back((keyLen >> 24) | 0x80);
            buffer.push_back((keyLen >> 16) & 0xFF);
            buffer.push_back((keyLen >> 8) & 0xFF);
            buffer.push_back(keyLen & 0xFF);
        }

        // 处理 value 长度
        if (valueLen < 128)
        {
            buffer.push_back(static_cast<uint8_t>(valueLen));
        }
        else
        {
            buffer.push_back((valueLen >> 24) | 0x80);
            buffer.push_back((valueLen >> 16) & 0xFF);
            buffer.push_back((valueLen >> 8) & 0xFF);
            buffer.push_back(valueLen & 0xFF);
        }

        // 添加 Key 和 Value
        buffer.insert(buffer.end(), key.begin(), key.end());
        buffer.insert(buffer.end(), value.begin(), value.end());
    }
    return buffer;
}
