/**
 * @file util.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-16
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef UTIL_H
#define UTIL_H

#include <unordered_map>
#include <string>
#include <sstream>
#include <vector>

#include "global.h"

using namespace std;

class Utils
{
public:
    static void ParseUrlencoded(std::string &data, std::unordered_map<std::string, std::string> &param);
    static int HexToDec(char ch);

    static string GetFileType(const string &path);
    static bool ParsePath(string &path);
    static string GetQuery(string &path);
};

inline void Utils::ParseUrlencoded(string &data, std::unordered_map<std::string, std::string> &param)
{
    if (data.size() == 0)
    {
        return;
    }
    std::string key, value;
    int pos = 0;
    int len = data.size();
    int i = 0;
    int tmp = 0;
    while (i < len)
    {
        char ch = data[i];
        switch (ch)
        {
        case '=':
            key = data.substr(pos, i - pos);
            pos = i + 1;
            break;
        case '+':
            data[i] = ' ';
            break;
        case '%':
            tmp = HexToDec(data[i + 1]) * 16 + HexToDec(data[i + 2]);
            data[i + 2] = tmp % 10 + '0';
            data[i + 1] = tmp / 10 + '0';
            i += 2;
        case '&':
            value = data.substr(pos, i - pos);
            pos = i + 1;
            param[key] = value;
            break;
        default:
            break;
        }
        i++;
    }
    if (pos < len)
    {
        value = data.substr(pos, i - pos);
        param[key] = value;
    }
}

inline int Utils::HexToDec(char ch)
{
    if (ch >= '0' && ch <= '9')
    {
        return ch - '0';
    }
    if (ch >= 'a' && ch <= 'f')
    {
        return ch - 'a' + 10;
    }
    if (ch >= 'A' && ch <= 'F')
    {
        return ch - 'A' + 10;
    }
    return ch;
}

inline string Utils::GetFileType(const string &path)
{
    string::size_type idx = path.find_last_of('.');
    if (idx == string::npos)
    {
        return "text/plain";
    }
    string suffix = path.substr(idx);
    if (suffix_des.count(suffix) == 1)
    {
        return suffix_des.find(suffix)->second;
    }
    return "text/plain";
}

inline string Utils::GetQuery(string &path)
{
    size_t queryStart = path.find('?');
    std::string query = path.substr(queryStart + 1);
    path = path.substr(0, queryStart);
    return query;
}

inline bool Utils::ParsePath(string &path)
{
    if (path == "/")
    {
        path += "login.html";
        return false;
    }
    else
    {
        for (auto &item : DEFAULT_HTML)
        {
            if (item == path)
            {
                path += ".html";
                return false;
            }
        }
    }
    return true;
}

#endif // UTIL_H__