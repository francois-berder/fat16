cmake_minimum_required(VERSION 2.8)

project(fs)

file(GLOB_RECURSE source_files src/*.c)

add_executable(fs ${source_files})
