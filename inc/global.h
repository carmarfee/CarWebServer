/**
 * @file global.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef GLOBAL_H
#define GLOBAL_H
#include <string>
#include <unordered_map>
#include <unordered_set>
using namespace std;

/************************************** HTTP  **************************************/


const std::unordered_map<std::string, std::string> suffix_des = {
    {".html", "text/html"},
    {".xml", "text/xml"},
    {".xhtml", "application/xhtml+xml"},
    {".txt", "text/plain"},
    {".rtf", "application/rtf"},
    {".pdf", "application/pdf"},
    {".word", "application/nsword"},
    {".png", "image/png"},
    {".gif", "image/gif"},
    {".jpg", "image/jpeg"},
    {".jpeg", "image/jpeg"},
    {".au", "audio/basic"},
    {".mpeg", "video/mpeg"},
    {".mpg", "video/mpeg"},
    {".avi", "video/x-msvideo"},
    {".gz", "application/x-gzip"},
    {".tar", "application/x-tar"},
    {".css", "text/css "},
    {".js", "text/javascript "},
}; // 文件后缀描述

const std::unordered_map<int, string> code_des = {
    {200, "GET_REQUEST"},
    {400, "BAD_REQUEST"},
    {403, "FORBIDDEN_REQUEST"},
    {404, "NO_RESOURCE"},
    {500, "INTERNAL_ERROR"},
}; // 状态码描述

const std::unordered_map<int, string> code_path = {
    {400, "/400.html"},
    {403, "/403.html"},
    {404, "/404.html"},
};

enum PARSE_STATE
{
    REQUEST_LINE,
    HEADERS,
    BODY,
    FINISH,
}; // 解析状态

const unordered_set<string> DEFAULT_HTML{
    "/index",
    "/register",
    "/login",
    "/welcome",
    "/video",
    "/picture",
};

// 请求报文
struct RequestMsg
{
    // HTTP请求内容
    std::string request_line;                // 请求行
    std::vector<std::string> request_header; // 请求报头
    char blank = '\n';                       // 空行
    std::string request_body;                // 请求正文
};

// 响应报文
struct ResponseMsg
{
    // HTTP响应内容
    std::string response_line;                // 响应行
    std::vector<std::string> response_header; // 响应报头
    char blank = '\n';                        // 空行
    std::string response_body;                // 响应正文
};

/************************************** FCGI **************************************/

#define FCGI_VERSION_1 1
#define FCGI_BEGIN_REQUEST 1
#define FCGI_PARAMS 4
#define FCGI_STDIN 5
#define FCGI_RESPONDER 1


// 8字节消息头
struct Fcgiheader
{
    /* data */
    uint8_t version;
    uint8_t type;
    uint16_t requestId;
    uint16_t contentLength;
    uint8_t paddingLength; // 消息体的长度始终是8字节的整数倍,实际内容长度不足时进行padding
    uint8_t reserved;
};

struct FcgiBeginRequest
{
    /* data */
    uint16_t role;
    uint8_t flags;
    uint8_t reserved[5];
};


#endif // GLOBAL_H
