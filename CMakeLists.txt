cmake_minimum_required(VERSION 3.30)
project(cpp-prefix-optimized-radix CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add source files
set(SOURCES
    radix/tree.cpp
    radix/node.cpp
    radix/iterator.cpp
)

# Add header files
set(HEADERS
    radix/tree.hpp
    radix/node.hpp
)

# Build the main executable
add_executable(iradix-cpp main.cpp ${SOURCES} ${HEADERS})

# Set output directory
set_target_properties(iradix-cpp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Add compiler flags
if(MSVC)
    target_compile_options(iradix-cpp PRIVATE /W4)
else()
    target_compile_options(iradix-cpp PRIVATE -Wall -Wextra -Wpedantic)
endif()