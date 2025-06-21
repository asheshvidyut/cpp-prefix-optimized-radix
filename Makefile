# Simple Makefile for cpp-prefix-optimized-radix project

CXX = g++
CXXFLAGS = -std=c++17 -O3 -DNDEBUG -Wall -Wextra
INCLUDES = -I. -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lbenchmark -labsl_strings -labsl_strings_internal -labsl_string_view -labsl_base -labsl_int128 -labsl_throw_delegate -labsl_raw_logging_internal -labsl_log_severity

# Source files
MAIN_SOURCES = main.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp
FUZZY_SOURCES = fuzzy_test_main.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp
BENCHMARK_SOURCES = benchmark.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp

# Targets
all: iradix-cpp fuzzy-test benchmark

iradix-cpp: $(MAIN_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

fuzzy-test: $(FUZZY_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

benchmark: $(BENCHMARK_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

clean:
	rm -f iradix-cpp fuzzy-test benchmark

.PHONY: all clean