cmake_minimum_required(VERSION 3.30)
project(rsquare CXX)

set(CMAKE_CXX_STANDARD 17)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pg -g -fPIC -O3 -std=gnu99 -Wall")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -pg")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -pg")

include_directories(${CMAKE_SOURCE_DIR}/include)

# Example: Build an executable named test_radix
add_executable(test_radix
        radix/main.cpp
        # Make sure to include the .cpp files (or .c files)
        # that define Tree_New, Tree_Insert, Tree_Root, etc.
        radix/radix.cpp
        radix/node.cpp
        radix/iterator.cpp
        radix/rev_iterator.cpp
)

add_library(rsquare MODULE
        radix/radix.cpp
        radix/node.cpp
        radix/iterator.cpp
        radix/rev_iterator.cpp
        rsquare.c
)

set_target_properties(rsquare PROPERTIES PREFIX "" SUFFIX ".so")

