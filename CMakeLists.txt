cmake_minimum_required(VERSION 2.8)

project(fs)

set(CMAKE_C_FLAGS_RELEASE "-Wall -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-Wall -ggdb3")

file(GLOB_RECURSE source_files src/*.c)

add_executable(fs ${source_files})
