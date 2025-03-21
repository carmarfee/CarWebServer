/**
 * @file buffer.h
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-02
 *
 * @copyright Copyright (c) 2025
 *
 */

#ifndef __BUFFER_H__
#define __BUFFER_H__

#include <cstring>   //提供了处理C风格字符串的函数，例如 strcpy, strlen, strcat 等等。perror 函数也在这个头文件中，用于打印描述错误代码的字符串。
#include <iostream>  //提供了标准输入输出流对象 cout 和 cin。
#include <unistd.h>  //提供了 POSIX 操作系统 API 的访问功能，例如 POSIX 文件操作函数（open, read, write, close 等等）。
#include <sys/uio.h> //提供了 readv 和 writev 函数，用于在一个调用中读取或写入多个缓冲区。
#include <vector>    //提供了一个动态数组容器，可以在运行时调整大小。
#include <atomic>    //提供了原子操作的支持，例如 std::atomic。
#include <assert.h>  //提供了一个宏 assert，用于在程序中插入调试断言。

class Buffer
{
public:
    Buffer(int initbuffsize = 1024);
    ~Buffer() = default;

    size_t WritableChar() const;
    size_t ReadableBytes() const;

    void HasWritten(size_t len);
    size_t HasRead() const;

    char *WritePosAddr();
    const char *WritePosAddrConst() const;
    const char *ReadPosAddr() const;

    void EnsureWriteable(size_t len);

    ssize_t ReadFd(int fd, int *Errno);
    ssize_t WriteFd(int fd, int *Errno);

    void Append(const char *str, size_t len);
    void Append(const void *data, size_t len);
    void Append(const std::string &str);
    void Append(const Buffer &buff);

    void Retrieve(size_t len);
    void RetrieveUntil(const char *end);

    void RetrieveAll();
    std::string RetrieveAllToStr();

    void Dropbytes(size_t len);
    void ResetBuffer(size_t len); // 如果buffer写不下从fd中读取的char,则追加extrabuff

private:
    char *BeginPtr_();
    const char *BeginPtr_() const;

private:
    std::vector<char> buffer_;
    std::atomic<std::size_t> readPos_;
    std::atomic<std::size_t> writePos_;
};

#endif // __BUFFER_H__
