# 编译器
CXX = g++

# 编译选项
CXXFLAGS = -std=c++11 -Wall -g

INCLUDES = -I../buffer -I.

# 目标文件
TARGET = test

# 源文件
SRCS = test.cpp log.cpp ../buffer/buffer.cpp


# 生成的目标文件
OBJS = $(SRCS:.cpp=.o)

# 规则：生成目标文件
$(TARGET): $(OBJS)
	$(CXX) $(CXXFLAGS) -o $@ $^

# 规则：生成目标文件的依赖文件
%.o: %.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) -c -o $@ $<

# 规则：清理生成的文件
.PHONY: clean
clean:
	rm -f $(TARGET) $(OBJS)