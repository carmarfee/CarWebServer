#!/bin/bash

cd "$(dirname "$0")"
make clean
make all

# 编译 backend 下的所有 .cpp 文件为 .cgi 可执行程序
backend_dir="./backend"
output_dir="./backend"
mkdir -p "$output_dir" # 确保输出目录存在

for cpp_file in "$backend_dir"/*.cpp; do
    if [ -f "$cpp_file" ]; then
        output_file="$output_dir/$(basename "${cpp_file%.cpp}.cgi")"
        g++  "$cpp_file" -o "$output_file" -lfcgi -lmysqlclient
        echo "Compiled $cpp_file -> $output_file"
    fi
done

# 获取所有运行的 .cgi 进程并终止它们
echo "Stopping all running .cgi processes..."
cgi_pids=$(pgrep -f '\.cgi')
if [ -n "$cgi_pids" ]; then
    echo "$cgi_pids" | xargs kill -9
    echo "Stopped the following .cgi processes: $cgi_pids"
else
    echo "No .cgi processes found."
fi

# 启动 spawn-fcgi 并添加生成的 .cgi 程序
for cgi_file in "$output_dir"/*.cgi; do
    if [ -f "$cgi_file" ]; then
        echo "Starting spawn-fcgi for $cgi_file..."
        spawn-fcgi -a 127.0.0.1 -p 10000 -f "$cgi_file"
    fi
done

echo "Build and deployment completed."