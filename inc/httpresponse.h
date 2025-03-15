/**
 * @file httpresponse.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-14
 *
 * @copyright Copyright (c) 2025
 *
 */

#include <unordered_map>
#include <fcntl.h>    // open
#include <unistd.h>   // close
#include <sys/stat.h> // stat
#include <sys/mman.h> // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"

class HttpResponse
{
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string &srcDir, std::string &path, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer &buff);
    void UnmapFile();
    char *File();
    size_t FileLen() const;
    void ErrorContent(Buffer &buff, std::string message);
    int Code() const { return code_; }

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;

    char *mmFile_;
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
};