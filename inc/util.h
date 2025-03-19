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

class Utils
{
public:
    static void ParseUrlencoded(std::string &data, std::unordered_map<std::string, std::string> &param);
    static int HexToDec(char ch);

    static string GetFileType(const string &path);
    static string ParsePath(string &path);

};

#endif // UTIL_H__