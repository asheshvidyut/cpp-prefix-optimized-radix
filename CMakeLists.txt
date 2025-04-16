cmake_minimum_required(VERSION 3.30)
project(cpp-immutable-radix CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add source files
set(SOURCES
    main.cpp
    iradix/tree.cpp
    iradix/node.cpp
    iradix/iterator.cpp
)

# Add header files
set(HEADERS
    iradix/tree.hpp
    iradix/node.hpp
    iradix/iterator.hpp
)

# Build the executable
add_executable(iradix-cpp ${SOURCES} ${HEADERS})

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