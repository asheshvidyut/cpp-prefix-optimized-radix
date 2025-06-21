# Simple Makefile for cpp-immutable-radix project

CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -Wpedantic
TARGET = x
SOURCES = main.cpp iradix/node.cpp iradix/tree.cpp iradix/iterator.cpp

# Default target
all: main

# Build the main executable
main: $(SOURCES)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(TARGET)

# Clean build artifacts
clean:
	rm -f $(TARGET)

.PHONY: all main clean