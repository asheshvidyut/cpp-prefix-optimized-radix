# Simple Makefile for cpp-prefix-optimized-radix project

CXX = g++
CXXFLAGS = -std=c++17 -O3 -DNDEBUG -Wall -Wextra
INCLUDES = -I. -I/opt/homebrew/include
LIBS = -L/opt/homebrew/lib -lbenchmark -labsl_strings -labsl_strings_internal -labsl_string_view -labsl_base -labsl_int128 -labsl_throw_delegate -labsl_raw_logging_internal -labsl_log_severity

# Source files
MAIN_SOURCES = main.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp
FUZZY_SOURCES = fuzzy_test_main.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp
BENCHMARK_SOURCES = benchmark.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp
BENCHMARK_UUID_SOURCES = benchmark_uuid.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp
REVERSE_ITERATOR_SOURCES = test_reverse_iterator.cpp radix/node.cpp radix/tree.cpp radix/iterator.cpp

# Targets
all: radix-cpp benchmark benchmark-uuid test-reverse-iterator

radix-cpp: $(MAIN_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

benchmark: $(BENCHMARK_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

benchmark-uuid: $(BENCHMARK_UUID_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^ $(LIBS)

test-reverse-iterator: $(REVERSE_ITERATOR_SOURCES)
	$(CXX) $(CXXFLAGS) $(INCLUDES) -o $@ $^

clean:
	rm -f radix-cpp fuzzy-test benchmark benchmark-uuid test-reverse-iterator

.PHONY: all clean