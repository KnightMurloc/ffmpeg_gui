project(xdg)

cmake_minimum_required(VERSION 3.14)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

include_directories(.)

add_library(xdg xdg.cpp)
set_property(TARGET xdg PROPERTY CXX_STANDARD 17)