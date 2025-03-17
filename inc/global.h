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

#endif // GLOBAL_H
