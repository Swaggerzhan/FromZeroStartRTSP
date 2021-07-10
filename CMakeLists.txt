cmake_minimum_required(VERSION 3.5.1)
project(FromZeroStartRTSP)

set(CMAKE_CXX_STANDARD 14)

add_executable(FromZeroStartRTSP main.cpp
        include/tcp.h src/tcp.cpp include/udp.h src/udp.cpp
        include/rtsp.h src/rtsp.cpp include/Buffer.h src/Buffer.cpp)