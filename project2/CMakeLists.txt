cmake_minimum_required(VERSION 3.29)
project(SO2_proj2)

set(CMAKE_CXX_STANDARD 20)

add_executable(SO2_proj2
        server.cpp
        common.h
        )

add_executable(client
        client.cpp
        common.h)

# link with winsock2
target_link_libraries(SO2_proj2 ws2_32)
target_link_libraries(client ws2_32)