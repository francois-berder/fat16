cmake_minimum_required(VERSION 2.8)

project(fs)

set(CMAKE_C_FLAGS_RELEASE "-DNDEBUG -O2")
set(CMAKE_C_FLAGS_DEBUG "-ggdb3")
set(CMAKE_C_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-ggdb3")
set(CMAKE_CXX_FLAGS  "-std=c++11 -Wall -Wextra")

file(GLOB_RECURSE test_files test/*.cpp test/*.c)
file(GLOB_RECURSE source_files driver/*.c)


add_library(fat16_driver SHARED ${source_files})
add_executable(run_test ${test_files})
target_link_libraries(run_test fat16_driver)
