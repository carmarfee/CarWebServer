/**
 * @file log.h
 * @author carmarfee (3073640166@qq.com)
 * @brief 
 * @version 0.1
 * @date 2025-03-11
 * 
 * @copyright Copyright (c) 2025
 * 
 */

 #ifndef LOG_H
 #define LOG_H
 
 #include <mutex>
 #include <string>
 #include <thread>
 #include <sys/time.h>
 #include <string.h>
 #include <stdarg.h>           // vastart va_end
 #include <assert.h>
 #include <sys/stat.h>         //mkdir
 #include "blockqueue.h"
 #include "../buffer/buffer.h"
 
 class Log {
 public:
    void init(int level = 1, const char* path = "./log", const char* suffix = ".log", int maxQueueCapacity = 1024);
    static Log* GetInstance();

private:
    Log();
    virtual ~Log();
    void AsyncWriteLog();

 private:
    // members
     
    static const int LOG_PATH_LEN = 256;
    static const int LOG_NAME_LEN = 256;
    static const int MAX_LINES = 50000;

    const char* path_;
    const char* suffix_;

    
 };
 #endif // LOG_H