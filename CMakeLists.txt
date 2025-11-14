cmake_minimum_required(VERSION 3.20)
project(PhotoEditor)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(PhotoEditor WIN32
    src/main.cpp
)
