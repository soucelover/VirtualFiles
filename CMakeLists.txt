cmake_minimum_required(VERSION 3.5)

project(VirtualFiles)

add_executable(main src/main.cpp)
target_include_directories(main PUBLIC src)
