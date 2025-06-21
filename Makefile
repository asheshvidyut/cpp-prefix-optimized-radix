# Simple Makefile for cpp-prefix-optimized-radix project

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
TARGET = x
SOURCES = main.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp

# Default target
all: main

# Build the main executable
main: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)

.PHONY: all main clean