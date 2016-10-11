cmake_minimum_required(VERSION 2.8)

project(fs C)

set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -ggdb3")

file(GLOB_RECURSE source_files src/*.c)

add_library(fat16_driver SHARED fs ${source_files})
