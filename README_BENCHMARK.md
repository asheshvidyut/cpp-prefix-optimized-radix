# Radix Tree Benchmark

This benchmark compares the performance of our radix tree implementation with Abseil's `btree_map`, following the microbenchmarking best practices from the [Abseil Performance Guide](https://abseil.io/fast/75).

## Prerequisites

You need to install the following dependencies:

### Ubuntu/Debian:
```bash
sudo apt-get install libbenchmark-dev libabsl-dev
```

### macOS:
```bash
brew install google-benchmark abseil
```

### Building from source (if packages are not available):

#### Google Benchmark:
```bash
git clone https://github.com/google/benchmark.git
cd benchmark
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DBENCHMARK_ENABLE_GTEST_TESTS=OFF ..
make -j$(nproc)
sudo make install
```

#### Abseil:
```bash
git clone https://github.com/abseil/abseil-cpp.git
cd abseil-cpp
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DABSL_BUILD_TESTING=OFF ..
make -j$(nproc)
sudo make install
```

## Building the Benchmark

### Using Make:
```bash
make benchmark
```

### Using CMake:
```bash
mkdir build && cd build
cmake ..
make benchmark
```

## Running the Benchmark

### Basic run:
```bash
./benchmark
```

### Run with specific benchmarks:
```bash
# Run only insertion benchmarks
./benchmark --benchmark_filter="Insert"

# Run only lookup benchmarks
./benchmark --benchmark_filter="Lookup"

# Run only iteration benchmarks
./benchmark --benchmark_filter="Iterate"
```

### Run with performance counters (Linux):
```bash
# Run with CPU cycles and cache misses
./benchmark --benchmark_counters_tabular=true --benchmark_perf_counters=cycles,cache-misses
```

### Run with different data sizes:
```bash
# Run with different word list sizes
./benchmark --benchmark_repetitions=3 --benchmark_report_aggregates_only=true
```

## Benchmark Categories

The benchmark includes the following categories:

### 1. **Insertion Performance**
- `BM_RadixTreeInsert`: Insert all words into radix tree
- `BM_BTreeMapInsert`: Insert all words into btree_map

### 2. **Lookup Performance**
- `BM_RadixTreeLookup`: Lookup all words in radix tree
- `BM_BTreeMapLookup`: Lookup all words in btree_map

### 3. **Iteration Performance**
- `BM_RadixTreeIterate`: Iterate through all words in radix tree
- `BM_BTreeMapIterate`: Iterate through all words in btree_map

### 4. **Prefix Search Performance**
- `BM_RadixTreePrefixSearch`: Prefix search in radix tree (native feature)
- `BM_BTreeMapLowerBound`: Lower bound search in btree_map (closest equivalent)

### 5. **Cache Performance**
- `BM_RadixTreeRandomAccess`: Random access patterns (cache-unfriendly)
- `BM_BTreeMapRandomAccess`: Random access patterns in btree_map
- `BM_RadixTreeSequentialAccess`: Sequential access patterns (cache-friendly)
- `BM_BTreeMapSequentialAccess`: Sequential access patterns in btree_map

## Interpreting Results

### Key Metrics:
- **Time**: Wall clock time per operation
- **CPU**: CPU time per operation
- **Items/sec**: Throughput (operations per second)
- **Bytes/sec**: Memory bandwidth utilization

### Performance Patterns to Look For:

1. **Insertion**: Radix trees typically have higher insertion overhead due to node splitting
2. **Lookup**: Radix trees can be faster for string keys due to prefix compression
3. **Prefix Search**: This is where radix trees should excel over btree_map
4. **Cache Performance**: Sequential access should be faster than random access
5. **Memory Usage**: Radix trees often use less memory due to prefix compression

### Example Output Interpretation:
```
BM_RadixTreeLookup              15.2 ns         15.1 ns    46236585
BM_BTreeMapLookup               18.7 ns         18.6 ns    37542196
```
This shows the radix tree lookup is ~20% faster than btree_map lookup.

## Microbenchmarking Best Practices Applied

Following the [Abseil guide](https://abseil.io/fast/75), this benchmark implements:

1. **`benchmark::DoNotOptimize()`**: Prevents compiler from eliminating unused results
2. **Consistent work**: Each benchmark does the same amount of work per iteration
3. **Multiple access patterns**: Tests both sequential and random access
4. **Realistic data**: Uses actual words from words.txt
5. **Performance counters**: Can be enabled to measure CPU cycles, cache misses, etc.
6. **Proper metrics**: Reports items/second and bytes/second for throughput analysis

## Troubleshooting

### Common Issues:

1. **Library not found**: Make sure benchmark and abseil libraries are installed
2. **Compilation errors**: Check that C++17 is supported by your compiler
3. **Runtime errors**: Ensure words.txt exists in the current directory

### Performance Tips:

1. **Run in release mode**: Always use `-O3` optimization
2. **Disable CPU frequency scaling**: `sudo cpupower frequency-set -g performance`
3. **Run multiple times**: Use `--benchmark_repetitions=3` for more reliable results
4. **Use performance counters**: Enable hardware counters for detailed analysis

## References

- [Abseil Performance Guide](https://abseil.io/fast/75)
- [Google Benchmark Documentation](https://github.com/google/benchmark/blob/main/docs/user_guide.md)
- [Abseil BTree Documentation](https://abseil.io/docs/cpp/guides/container#btree) 


## Benchmarking Results

Loaded 235886 words
Radix tree size: 235886
BTree map size: 235886
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2025-06-21T09:00:05+05:30
Running ./benchmark
Run on (10 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 2.78, 2.24, 2.25
---------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------
BM_RadixTreeInsert            438361771 ns    438347500 ns            2 bytes_per_second=24.6334Mi/s items_per_second=538.126k/s
BM_BTreeMapInsert              15827585 ns     15811727 ns           44 bytes_per_second=682.911Mi/s items_per_second=14.9184M/s
BM_RadixTreeLookup             35092865 ns     35092800 ns           20 bytes_per_second=153.849Mi/s items_per_second=6.72178M/s
BM_BTreeMapLookup              21560661 ns     20840265 ns           34 bytes_per_second=259.066Mi/s items_per_second=11.3188M/s
BM_RadixTreeIterate                1097 ns         1097 ns       639801 bytes_per_second=9.38517Ti/s items_per_second=214.981G/s
BM_BTreeMapIterate               324153 ns       324154 ns         2146 bytes_per_second=32.5306Gi/s items_per_second=727.698M/s
BM_RadixTreePrefixSearch           6357 ns         6357 ns       109368 bytes_per_second=21.602Mi/s items_per_second=943.804k/s
BM_BTreeMapLowerBound             76756 ns        76756 ns         9000 bytes_per_second=1.78917Mi/s items_per_second=78.1701k/s
BM_RadixTreeRandomAccess         415951 ns       415952 ns         1677 bytes_per_second=55.0261Mi/s items_per_second=2.40413M/s
BM_BTreeMapRandomAccess          177730 ns       177730 ns         3735 bytes_per_second=128.78Mi/s items_per_second=5.6265M/s
BM_RadixTreeSequentialAccess   34953304 ns     34953300 ns           20 bytes_per_second=154.463Mi/s items_per_second=6.7486M/s
BM_BTreeMapSequentialAccess    20590301 ns     20590324 ns           34 bytes_per_second=262.211Mi/s items_per_second=11.4562M/s