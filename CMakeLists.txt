cmake_minimum_required(VERSION 3.27)
project(simplewm)

set(CMAKE_CXX_STANDARD 17)

add_executable(simplewm main.cpp
        window_manager.h
        window_manager.cpp
        util.cpp
        util.h
        structs.h
)
