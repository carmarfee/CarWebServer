/**
 * @file log.cpp
 * @author carmarfee (3073640166@qq.com)
 * @brief
 * @version 0.1
 * @date 2025-03-11
 *
 * @copyright Copyright (c) 2025
 *
 */

#include "../../inc/log.h"

using namespace std;

Log::Log()
{
    lineCount_ = 0;
    isopen_ = false;
    level_ = 1;
    isAsync_ = false;
    path_ = nullptr;
    suffix_ = nullptr;
    fp_ = nullptr;
}

Log::~Log()
{
    if (writeThread_ && writeThread_->joinable())
    {
        while (!logQueue_->empty()) // 将日志队列中的
        {
            logQueue_->flush();
        };
        logQueue_->close();
        writeThread_->join();
    }
    if (fp_)
    {
        lock_guard<mutex> locker(mtx_);
        flush();
        fclose(fp_);
    }
}

void Log::flush()
{
    if (isAsync_)
    {
        logQueue_->flush(); // 异步的话就唤醒一个消费者将日志队列中的数据全部读走
    }
    fflush(fp_);
}

void Log::AsyncWriteLog_()
{
    string str = "";
    while (logQueue_->pop(str))
    {
        lock_guard<mutex> locker(mtx_);
        fputs(str.c_str(), fp_);
    }
}

int Log::GetLevel()
{
    lock_guard<mutex> locker(mtx_);
    return level_;
}

void Log::SetLevel(int level)
{
    lock_guard<mutex> locker(mtx_);
    level_ = level;
}

void Log::init(int level, const char *path, const char *suffix, int maxQueueSize)
{
    level_ = level;
    isopen_ = true;
    if (maxQueueSize > 0)
    {
        isAsync_ = true;
        if (!logQueue_)
        {
            unique_ptr<blockqueue<string>> newLogQueue(new blockqueue<string>(maxQueueSize));
            logQueue_ = move(newLogQueue);

            unique_ptr<thread> NewWriteThread(new thread(LogThread));
            writeThread_ = move(NewWriteThread);
        }
    }
    else
    {
        isAsync_ = false;
    }
    lineCount_ = 0;

    time_t timer = time(nullptr);
    struct tm *sysTime = localtime(&timer);
    struct tm t = *sysTime;
    path_ = path;
    suffix_ = suffix;
    char fileName[LOG_NAME_LEN] = {0};
    snprintf(fileName, LOG_NAME_LEN - 1, "%s/%04d_%02d_%02d%s",
             path_, t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, suffix_);
    toDay_ = t.tm_mday;

    {
        lock_guard<mutex> locker(mtx_);
        buff_.RetrieveAll();
        if (fp_)
        {
            flush();
            fclose(fp_);
        }

        fp_ = fopen(fileName, "a");
        if (fp_ == nullptr)
        {
            mkdir(path_, 0777);
            fp_ = fopen(fileName, "a");
        }
        assert(fp_ != nullptr);
    }
}

Log *Log::GetInstance()
{
    static Log inst;
    return &inst;
}

void Log::LogThread()
{
    Log::GetInstance()->AsyncWriteLog_();
}

void Log::write(int level, const char *format, ...)
{
    struct timeval now = {0, 0};
    gettimeofday(&now, nullptr);
    time_t tSec = now.tv_sec;
    struct tm *sysTime = localtime(&tSec);
    struct tm t = *sysTime;
    va_list vaList;

    if (toDay_ != t.tm_mday || (lineCount_ && (lineCount_ % MAX_LINES == 0)))
    {
        unique_lock<mutex> locker(mtx_);
        locker.unlock();

        char newFile[LOG_NAME_LEN];
        char tail[36] = {0};
        snprintf(tail, 36, "%04d_%02d_%02d", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday);

        if (toDay_ != t.tm_mday)
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s%s", path_, tail, suffix_);
            toDay_ = t.tm_mday;
            lineCount_ = 0;
        }
        else
        {
            snprintf(newFile, LOG_NAME_LEN - 72, "%s/%s-%d%s", path_, tail, (lineCount_ / MAX_LINES), suffix_);
        }

        locker.lock();
        flush();
        fclose(fp_);
        fp_ = fopen(newFile, "a");
        assert(fp_ != nullptr);
    }

    {
        unique_lock<mutex> locker(mtx_);
        lineCount_++;
        int n = snprintf(buff_.WritePosAddr(), 128, "%d-%02d-%02d %02d:%02d:%02d.%06ld ",
                         t.tm_year + 1900, t.tm_mon + 1, t.tm_mday,
                         t.tm_hour, t.tm_min, t.tm_sec, now.tv_usec);

        buff_.HasWritten(n);
        AppendLogLevelTitle_(level);

        va_start(vaList, format);
        int m = vsnprintf(buff_.WritePosAddr(), buff_.WritableChar(), format, vaList);
        va_end(vaList);

        buff_.HasWritten(m);
        buff_.Append("\n\0", 2);

        if (isAsync_ && logQueue_ && !logQueue_->full())
        {
            logQueue_->push(buff_.RetrieveAllToStr());
        }
        else
        {
            fputs(buff_.ReadPosAddr(), fp_);
        }
        buff_.RetrieveAll();
    }
}

void Log::AppendLogLevelTitle_(int level)
{
    switch (level)
    {
    case 0:
        buff_.Append("[debug]: ", 9);
        break;
    case 1:
        buff_.Append("[info] : ", 9);
        break;
    case 2:
        buff_.Append("[warn] : ", 9);
        break;
    case 3:
        buff_.Append("[error]: ", 9);
        break;
    default:
        buff_.Append("[info] : ", 9);
        break;
    }
}