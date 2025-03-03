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
#include <iostream> //提供了标准输入输出流对象 cout 和 cin。
#include <unistd.h>  //提供了 POSIX 操作系统 API 的访问功能，例如 POSIX 文件操作函数（open, read, write, close 等等）。
#include <sys/uio.h> //提供了 readv 和 writev 函数，用于在一个调用中读取或写入多个缓冲区。
#include <vector> //提供了一个动态数组容器，可以在运行时调整大小。
#include <atomic> //提供了原子操作的支持，例如 std::atomic。
#include <assert.h> //提供了一个宏 assert，用于在程序中插入调试断言。


class Buffer {
public:
    Buffer(int initBuffSize = 1024);
    ~Buffer();

    size_t WritableBytes() const; //返回buffer中可写入的字节数
    size_t ReadableBytes() const; //返回buffer中可读入的字节数
    size_t PrependableBytes() const; //返回buffer中预备的字节数(指缓冲区头部的空闲空间)


    const char* Peek() const; //返回缓冲区中可读数据的起始地址
    void EnsureWriteable(size_t len); //确保缓冲区有足够的空间写下len个字节
    void HasWritten(size_t len); //已经写入len个字节
    void Retrieve(size_t len); //读出len个字节,如果len大于可读的字节数,则报错
    void RetrieveUntil(const char* end); //取回直到end的字节
    void RetrieveAll(); //取回所有字节
    std::string RetrieveAllToStr(); //取回所有字节并返回字符串
    const char* BeginWriteConst() const; //返回可写数据的起始地址
    char* BeginWrite(); //返回可写数据的起始地址
    void Append(const std::string& str); //追加字符串
    void Append(const char* /*restrict*/ data, size_t len); //追加数据
    void Append(const void* /*restrict*/ data, size_t len); //追加数据
    void Append(const Buffer& buff); //追加buffer
    ssize_t ReadFd(int fd, int* saveErrno); //从fd中读取数据
    ssize_t WriteFd(int fd, int* saveErrno); //向fd中写入数据

private:
    char* BeginPtr_(); //返回缓冲区的起始地址
    const char* BeginPtr_() const; //返回缓冲区的起始地址
    void MakeSpace_(size_t len); //确保缓冲区有足够的空间写下len个字节

    std::vector<char> buffer_; //缓冲区
    std::atomic<std::size_t> readPos_; //读位置
    std::atomic<std::size_t> writePos_; //写位置
};

#endif // __BUFFER_H__
