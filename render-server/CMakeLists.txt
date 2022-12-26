cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

project(ssr VERSION 1.0)
add_executable(ssr src/server.cpp)

add_definitions(-D_DEBUG)

include_directories(thirdparty)
