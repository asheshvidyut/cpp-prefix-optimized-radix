#!/bin/bash

# Run benchmark with memory tracking
echo "Running benchmark with memory tracking..."
echo "========================================"

# Run the benchmark
make clean
make all
./benchmark --benchmark_format=console --benchmark_counters_tabular=true

echo ""
echo "Memory usage summary:"
echo "===================="
echo "The benchmark now includes the following memory metrics:"
echo "- TotalMemory: Total memory used by the process after all insertions (main metric for total memory consumed)"
echo "- MemoryPeak: Peak memory usage during the benchmark"
echo "- RSSDelta: Change in Resident Set Size"
echo "- MemoryPerItem: Memory usage per item (for insert benchmarks)"
echo "- PeakPerItem: Peak memory usage per item (for insert benchmarks)"
echo ""
echo "To get more detailed memory analysis, you can also run:"
echo "./benchmark --benchmark_format=json > results.json"
echo "This will output detailed memory statistics in JSON format." 