# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++11 -Wall -g

# 包含路径
INCLUDES = -I./inc

# 目标文件
TARGET = server

# 源文件
SRCS = main.cpp \
	   src/buffer/buffer.cpp \
	   src/http/httpconn.cpp \
	   src/http/httprequest.cpp \
	   src/http/httpresponse.cpp \
	   src/log/log.cpp \
	   src/pool/sqlconnpool.cpp \
	   src/server/epoller.cpp \
	   src/server/webserver.cpp \
	   src/timer/heaptimer.cpp \

# 生成的目标文件
OBJS = $(SRCS:.cpp=.o)

# 规则：生成目标文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

# 规则：生成目标文件的依赖文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# 规则：清理生成的文件
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)