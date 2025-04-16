cmake_minimum_required(VERSION 3.30)
project(cpp-immutable-radix CXX)

set(CMAKE_CXX_STANDARD 17)

include_directories(${CMAKE_SOURCE_DIR})

# Example: Build an executable named iradix-cpp
add_executable(iradix-cpp
        iradix/main.cpp
        iradix/radix.cpp
        iradix/node.cpp
        iradix/iterator.cpp
)