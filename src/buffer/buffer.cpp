/**
 * @file buffer.c
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-02
 *
 * @copyright Copyright (c) 2025
 *
 */
#include "../../inc/buffer.h"

Buffer::Buffer(int initbuffSize) : buffer_(initbuffSize), readPos_(0), writePos_(0) {}

size_t Buffer::ReadableBytes() const
{
    return writePos_ - readPos_;
}
size_t Buffer::WritableChar() const
{
    return buffer_.size() - writePos_;
}

void Buffer::HasWritten(size_t len)
{
    writePos_ += len;
}

size_t Buffer::HasRead() const
{
    return readPos_;
}

void Buffer::ResetBuffer(size_t len)
{
    if (WritableChar() + HasRead() < len)
    {
        buffer_.resize(writePos_ + len + 1);
    }
    else
    {
        std::copy(BeginPtr_() + readPos_, BeginPtr_() + writePos_, BeginPtr_());
        readPos_ = 0;
        writePos_ = ReadableBytes();
    }
}

char *Buffer::WritePosAddr()
{
    return BeginPtr_() + writePos_;
}

const char *Buffer::WritePosAddrConst() const
{
    return BeginPtr_() + writePos_;
}

const char *Buffer::ReadPosAddr() const
{
    return BeginPtr_() + readPos_;
}

ssize_t Buffer::ReadFd(int fd, int *Errno)
{
    char buff[65535];
    struct iovec iov[2];
    const size_t writable = WritableChar();
    /* 分散读， 保证数据全部读完 */
    iov[0].iov_base = BeginPtr_() + writePos_;
    iov[0].iov_len = writable;
    iov[1].iov_base = buff;
    iov[1].iov_len = sizeof(buff);

    const ssize_t n = readv(fd, iov, 2);
    if (n < 0)
    {
        *Errno = errno;
    }
    else if (static_cast<size_t>(n) <= writable)
    {
        writePos_ += n;
    }
    else
    {
        writePos_ = buffer_.size();
        Append(buff, n - writable);
    }
    return n;
}

ssize_t Buffer::WriteFd(int fd, int *Errno)
{
    size_t readable = ReadableBytes();
    ssize_t n = write(fd, ReadPosAddr(), readable);
    if (n < 0)
    {
        *Errno = errno;
    }
    readPos_ += n;
    return n;
}

void Buffer::Append(const std::string &str)
{
    Append(str.data(), str.length());
}

void Buffer::Append(const void *data, size_t len)
{
    assert(data);
    Append(static_cast<const char *>(data), len);
}

void Buffer::Append(const char *str, size_t len)
{
    assert(str);
    EnsureWriteable(len);
    std::copy(str, str + len, WritePosAddr());
    HasWritten(len);
}

void Buffer::Append(const Buffer &buff)
{
    Append(buff.ReadPosAddr(), buff.ReadableBytes());
}

void Buffer::EnsureWriteable(size_t len)
{
    if (WritableChar() < len)
    {
        ResetBuffer(len);
    }
}

void Buffer::Retrieve(size_t len)
{
    assert(ReadableBytes() >= len);
    readPos_ += len;
}

void Buffer::RetrieveUntil(const char *end)
{
    assert(ReadPosAddr() <= end);
    Retrieve(end - ReadPosAddr());
}

void Buffer::RetrieveAll()
{
    bzero(&buffer_[0], buffer_.size());
    readPos_ = 0;
    writePos_ = 0;
}

std::string Buffer::RetrieveAllToStr()
{
    std::string str(ReadPosAddr(), ReadableBytes());
    RetrieveAll();
    return str;
}

void Buffer::Dropbytes(size_t len)
{
    assert(ReadableBytes() >= len);
    writePos_ -= len;;
}

char *Buffer::BeginPtr_()
{
    return &*buffer_.begin();
}

const char *Buffer::BeginPtr_() const
{
    return &*buffer_.begin();
}
