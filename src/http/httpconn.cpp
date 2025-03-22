#include "../../inc/httpconn.h"
using namespace std;

bool HttpRequest::ParseRequestLine_(const string &line)
{
    std::istringstream is(line);
    requestmsg_.request_line = line;
    is >> method_ >> path_ >> version_;
    std::transform(method_.begin(), method_.end(), method_.begin(), ::toupper);
    if (method_.empty() || path_.empty() || version_.empty())
    {
        LOG_ERROR("RequestLine error");
        return false;
    }
    state_ = HEADERS;
    return true;
}

bool HttpRequest::ParseHeader_(const string &headers)
{
    size_t pos = headers.find(":");
    if (pos == string::npos)
    {
        state_ = BODY;
        return true;
    }
    requestmsg_.request_header.push_back(headers);
    string key = headers.substr(0, pos);
    string value = headers.substr(pos + 2);
    header_kv_[key] = value;
    return true;
}

bool HttpRequest::ParseBody_(const string &body)
{
    requestmsg_.request_body = body;
    query_string_ = Utils::GetQuery(path_);
    cgi_ = Utils::ParsePath(path_);
    state_ = FINISH; // 如果不是cgi请求，则返回请求的html页面
    return true;
}

bool HttpRequest::ParseRequestMsg(Buffer &buff)
{
    const char CRLF[] = "\r\n";
    if (buff.ReadableBytes() <= 0)
    {
        return false;
    }
    while (buff.ReadableBytes() && state_ != FINISH)
    {
        const char *lineEnd = search(buff.ReadPosAddr(), buff.WritePosAddrConst(), CRLF, CRLF + 2);
        std::string line(buff.ReadPosAddr(), lineEnd);
        switch (state_)
        {
        case REQUEST_LINE:
            if (!ParseRequestLine_(line))
            {
                return false;
            }
            break;
        case HEADERS:
            ParseHeader_(line);
            if (buff.ReadableBytes() <= 2) // 读到最后的/r/n,而不是/r/n/r/n，说明后面没有body
            {
                query_string_ = Utils::GetQuery(path_);
                cgi_ = Utils::ParsePath(path_);
                state_ = FINISH;
            }
            break;
        case BODY:
            ParseBody_(line);
            break;
        default:
            break;
        }
        if (lineEnd == buff.WritePosAddr()) // buff读完了直接退出解析loop
        {
            break;
        }
        buff.RetrieveUntil(lineEnd + 2);
    }
    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

void HttpResponse::AddResponseLine_(Buffer &buff)
{
    string status;
    if (code_des.count(code_) == 1)
    {
        status = code_des.find(code_)->second;
    }
    else
    {
        code_ = 400;
        status = code_des.find(code_)->second;
    }
    responsemsg_.response_line = "HTTP/1.1 " + to_string(code_) + " " + status + "\r\n";
    buff.Append(responsemsg_.response_line);
}

void HttpResponse::AddResponseHeader_(Buffer &buff)
{
    if (isKeepAlive_)
    {
        responsemsg_.response_header.push_back("Connection: keep-alive\r\n");
        responsemsg_.response_header.push_back("Keep-Alive: max=6, timeout=120\r\n");
    }
    else
    {
        responsemsg_.response_header.push_back("Connection: close\r\n");
    }
    std::string filetype = Utils::GetFileType(path_);
    responsemsg_.response_header.push_back("Content-type: " + filetype + "\r\n");

    buff.Append("Connection: keep-alive\r\n");
    buff.Append("Keep-Alive: max=6, timeout=120\r\n");
    buff.Append("Connection: close\r\n");
    buff.Append("Content-type: " + filetype + "\r\n");
}

void HttpResponse::AddResponseBody_(Buffer &buff)
{
    int srcFd = open((srcDir_ + path_).c_str(), O_RDONLY);
    if (srcFd < 0)
    {
        LOG_ERROR("Open file error");
        code_ = 404;
        ErrorContent("Open file error");
        buff.Append(responsemsg_.response_header.back());
        return;
    }
    int *mmapRet = (int *)mmap(0, file_stat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
    if (*mmapRet == -1)
    {
        LOG_ERROR("mmap error");
        code_ = 404;
        ErrorContent("mmap error");
        return;
    }
    file_content_ = (char *)mmapRet;
    responsemsg_.response_body = file_content_;
    close(srcFd);
}

void HttpResponse::MakeResponseMsg(Buffer &buff)
{
    LOG_DEBUG("filepath:%s", (srcDir_ + path_).c_str());
    if (stat((srcDir_ + path_).data(), &file_stat_) < 0 || S_ISDIR(file_stat_.st_mode))
    {
        code_ = 404;
    }
    else if (!(file_stat_.st_mode & S_IROTH))
    {
        code_ = 403;
    }
    else if (code_ == -1)
    {
        code_ = 200;
    }
    if (code_path.count(code_) == 1)
    {
        path_ = code_path.find(code_)->second;
        stat((srcDir_ + path_).data(), &file_stat_);
    }
    AddResponseLine_(buff);
    AddResponseHeader_(buff);
    AddResponseBody_(buff);
}

void HttpResponse::ErrorContent(string message)
{
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if (code_des.count(code_) == 1)
    {
        status = code_des.find(code_)->second;
    }
    else
    {
        status = code_des.find(400)->second;
    }
    body += to_string(code_) + " : " + status + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    responsemsg_.response_body = body;
}

void HttpResponse::Close()
{
    if (file_content_)
    {
        munmap(file_content_, file_stat_.st_size);
        file_content_ = nullptr;
    }
}

const char *HttpConn::srcDir;
std::atomic<int> HttpConn::userCount;

void HttpConn::Init(int fd, const sockaddr_in &addr)
{
    assert(fd > 0);
    userCount++;
    addr_ = addr;
    fd_ = fd;
    writeBuff_.RetrieveAll();
    readBuff_.RetrieveAll();
    isClose_ = false;
    LOG_INFO("Client[%d](%s:%d) in, userCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
}

ssize_t HttpConn::Read(int *saveErrno)
{
    int len = -1;
    while (true)
    {
        len = readBuff_.ReadFd(fd_, saveErrno);
        if (len <= 0)
        {
            break;
        }
    }
    return len;
}

ssize_t HttpConn::Write(int *saveErrno)
{
    ssize_t len = -1;
    while (true)
    {
        len = writev(fd_, iov_, iovCnt_);
        if (len <= 0)
        {
            *saveErrno = errno;
            break;
        }
        if (iov_[0].iov_len + iov_[1].iov_len == 0)
        {
            break;
        } /* 传输结束 */
        else if (static_cast<size_t>(len) > iov_[0].iov_len)
        {
            iov_[1].iov_base = (uint8_t *)iov_[1].iov_base + (len - iov_[0].iov_len);
            iov_[1].iov_len -= (len - iov_[0].iov_len);
            if (iov_[0].iov_len)
            {
                writeBuff_.RetrieveAll();
                iov_[0].iov_len = 0;
            }
        }
        else
        {
            iov_[0].iov_base = (uint8_t *)iov_[0].iov_base + len;
            iov_[0].iov_len -= len;
            writeBuff_.Retrieve(len);
        }
    }
    return len;
}

bool HttpConn::Process()
{
    request_.Init();
    if (readBuff_.ReadableBytes() <= 0)
    {
        return false;
    }
    else if (request_.ParseRequestMsg(readBuff_))
    {
        std::string request_path = request_.GetPath();
        response_.Init(srcDir, request_path, request_.IsKeepAlive(), 200);
    }
    else
    {
        std::string request_path = request_.GetPath();
        response_.Init(srcDir, request_path, false, 400);
    }

    response_.MakeResponseMsg(writeBuff_);
    if (IsCgi())
    {
        cgiserver_.ConnectFcgiServer();
        cgiserver_.MakeFcgiRequest(srcDir, request_.query_string_, request_.GetMethod(), request_.GetPath());
        cgiserver_.SendFcgiRequset();
        cgiserver_.ReadandParseFcgiResponse(writeBuff_);

        /* 响应头+响应体 */
        iov_[0].iov_base = const_cast<char *>(writeBuff_.ReadPosAddr());
        iov_[0].iov_len = writeBuff_.ReadableBytes();
        iovCnt_ = 1;
    }
    else
    {
        /* 响应头 */
        iov_[0].iov_base = const_cast<char *>(writeBuff_.ReadPosAddr());
        iov_[0].iov_len = writeBuff_.ReadableBytes();
        iovCnt_ = 1;
        /* 响应体 from plain or html*/
        if (response_.GetContent())
        {
            if (response_.GetContentLength(1) > 0) // html file
            {
                iov_[1].iov_base = response_.GetContent(1);
                iov_[1].iov_len = response_.GetContentLength(1);

                /* 添加content_length */
                writeBuff_.Append("Content-Length: " + to_string(iov_[1].iov_len) + "\r\n\r\n");

                iovCnt_ = 2;
            }
            else if (response_.GetContentLength(0) > 0) // plain content
            {
                iov_[1].iov_base = response_.GetContent(0);
                iov_[1].iov_len = response_.GetContentLength(0);

                /* 添加content_length */
                writeBuff_.Append("Content-Length: " + to_string(iov_[1].iov_len) + "\r\n\r\n");

                iovCnt_ = 2;
            }
        }
    }
    return true;
}

void HttpConn::Close()
{
    response_.Close();
    if (isClose_ == false)
    {
        isClose_ = true;
        userCount--;
        close(fd_);
        LOG_INFO("Client[%d](%s:%d) quit, UserCount:%d", fd_, GetIP(), GetPort(), (int)userCount);
    }
}
