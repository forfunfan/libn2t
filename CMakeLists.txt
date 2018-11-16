cmake_minimum_required(VERSION 3.0)
project(libn2t)
add_library(n2t src/lwip/src/core/init.c)
include_directories(
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/lwip/src/include
)
