cmake_minimum_required(VERSION 3.5)

project(VirtualFiles)

add_executable(main "src/main.cpp" "src/file_path.h" "src/file_entries.h" "src/virt_exceptions.h" "src/virt_filebuf.h" "src/virt_fstream.h")
