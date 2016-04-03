cmake_minimum_required(VERSION 3.3)
project(Mini_FTP)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    common.c
    common.h
    miniftp.c
    miniftpd.c
    requests.h)

add_executable(Mini_FTP ${SOURCE_FILES})