cmake_minimum_required(VERSION 2.8)

project(fs)

set(CMAKE_C_FLAGS_RELEASE "-Wall -Wextra -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG "-Wall -Wextra -ggdb3")
set(CMAKE_CXX_FLAGS_RELEASE "-std=c++11 -Wall -Wextra -DNDEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "-std=c++11 -Wall -Wextra -ggdb3")

file(GLOB_RECURSE test_files test/*.cpp test/*.c)
file(GLOB_RECURSE source_files driver/*.c)


add_library(fat16_driver SHARED ${source_files})
add_executable(run_test ${test_files})
target_link_libraries(run_test fat16_driver)
