# CarWebServer 🚀
---
CarWebServer 是一个轻量级的基于tinyWebserver的服务器，基于 C++ 实现，旨在用于学习webserver的架构和实现。它支持基本的 HTTP 请求处理、静态文件服务以及并发连接管理，适合用于学习网络编程、多线程编程和服务器开发。

### 项目特点 ✨
- 轻量级：代码简洁,目录结构清晰.易于理解和扩展。

- 高性能：基于多线程和 I/O 多路复用技术，支持高并发。

- 模块化设计：各个功能模块清晰分离，便于维护和扩展。

- 部署平台：适用于Linux 环境，易于部署和测试。

- HTTP/1.1 支持：支持 GET 和 POST 请求，能够处理静态文件请求。

- Todo：websocket连接,更多类型的request请求服务...


### 快速开始 🛠️
- 操作系统：Linux

- 编译器：GCC (支持 C++11 或更高版本)

- 其他依赖：   
```sudo apt-get install libmysqlclient-dev ```

- 编译源码： 
    在工作目录下执行
```make```

- 启动服务器： 
```./server```

- 访问服务器： 
    本地浏览器输入
```localhost:1316(默认端口为1316)```

### 致谢 🙏
    感谢 CSDN 和 GitHub 提供的学习资源。原项目tinywebserver:https://github.com/qinguoyi/TinyWebServer


