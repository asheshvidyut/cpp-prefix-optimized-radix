cmake_minimum_required(VERSION 3.16)
project(cpp-prefix-optimized-radix)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(benchmark REQUIRED)
find_package(absl REQUIRED)

# Add include directories
include_directories(${CMAKE_SOURCE_DIR})

# Add source files
set(SOURCES
    main.cpp
    fuzzy_test_main.cpp
    radix/node.cpp
    radix/tree.cpp
    radix/iterator.cpp
    benchmark.cpp
)

# Add header files
set(HEADERS
    radix/tree.hpp
    radix/node.hpp
)

# Build the main executable
add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})

# Set output directory
set_target_properties(${PROJECT_NAME} PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Link libraries
target_link_libraries(${PROJECT_NAME} 
    benchmark::benchmark
    absl::base
    absl::container
    absl::strings
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Compiler flags
target_compile_options(${PROJECT_NAME} PRIVATE
    -O3
    -DNDEBUG
)