# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++14 -Wall -g

# 包含路径
INCLUDES = -I./inc

# 目标文件
TARGET = server

# 源文件
SRCS = main.cpp \
       src/buffer/buffer.cpp \
       src/http/httpconn.cpp \
       src/log/log.cpp \
       src/pool/sqlconnpool.cpp \
       src/server/epoller.cpp \
       src/server/webserver.cpp \
       src/timer/heaptimer.cpp \
       src/util/util.cpp \
       src/fastcgi/fastcgi.cpp \

# 生成的目标文件
OBJS = $(SRCS:.cpp=.o)

# 默认目标：启动 PHP-FPM 并编译 C++ 项目
all: start-phpfpm $(TARGET)
	@echo "PHP-FPM started and C++ project compiled."

# 规则：生成目标文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ -pthread -lmysqlclient

# 规则：生成目标文件的依赖文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# 启动 PHP-FPM
start-phpfpm:
	sudo systemctl start php8.3-fpm
	@echo "PHP-FPM started."

# 停止 PHP-FPM
stop-phpfpm:
	sudo systemctl stop php8.3-fpm
	@echo "PHP-FPM stopped."

# 清理生成的文件并停止 PHP-FPM
.PHONY: clean
clean: stop-phpfpm
	rm -f $(TARGET) $(OBJS)
	@echo "PHP-FPM stopped and project cleaned up."