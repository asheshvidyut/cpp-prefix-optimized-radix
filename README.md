# CPP Prefix Optimized Radix Tree

A C++ implementation of a prefix-optimized radix tree with leaf-based iteration, inspired by the Go implementation at https://github.com/asheshvidyut/prefix-search-optimized-radix/.

## Features

- **Mutable Operations**: Insert, delete, and update operations modify nodes in place
- **Leaf-Based Iteration**: Efficient iteration using leaf linking for O(1) next/prev operations
- **Prefix Compression**: Shared prefixes are stored once to reduce memory usage
- **Generic Design**: Template-based implementation supporting any key-value types
- **Comprehensive Benchmarking**: Performance comparison with Abseil's btree_map

## Quick Start

### Prerequisites

#### macOS:
```bash
brew install google-benchmark abseil
```

### Build
```bash
make all
```

### Run Benchmarks
```bash
make benchmark
./benchmark --benchmark_filter="Iterate"
```

## Project Structure

```
cpp-prefix-optimized-radix/
├── radix/
│   ├── tree.hpp      # Main radix tree implementation
│   ├── tree.cpp      # Tree implementation
│   ├── node.hpp      # Node structure and operations
│   ├── node.cpp      # Node implementation
│   ├── iterator.cpp  # Leaf-based iterator
├── main.cpp          # Example usage
├── benchmark.cpp     # Performance benchmarks
└── words.txt         # Test data
```

## Key Design Decisions

1. **Mutable Operations**: Nodes are modified in place during insert/delete operations
2. **Leaf Linking**: Each node maintains `minLeaf`, `maxLeaf`, and `leaves_in_subtree` properties
3. **Efficient Iteration**: Iterator uses leaf links for O(1) next/prev operations
4. **Memory Optimization**: Prefix compression reduces memory usage for similar keys

## Benchmarking

This project includes comprehensive benchmarks comparing the radix tree with Abseil's `btree_map`.

### Running Benchmarks

#### Run all benchmarks:
```bash
./benchmark
./benchmark-uuid
```

#### Run specific benchmarks:
```bash
# Run only insertion benchmarks
./benchmark --benchmark_filter="Insert"

# Run only lookup benchmarks
./benchmark --benchmark_filter="Lookup"

# Run only iteration benchmarks
./benchmark --benchmark_filter="Iterate"
```

### Benchmark Categories

1. **Insertion Performance**
   - `BM_RadixTreeInsert`: Insert all words into radix tree
   - `BM_BTreeMapInsert`: Insert all words into btree_map

2. **Lookup Performance**
   - `BM_RadixTreeLookup`: Lookup all words in radix tree
   - `BM_BTreeMapLookup`: Lookup all words in btree_map

3. **Iteration Performance**
   - `BM_RadixTreeIterate`: Iterate through all words in radix tree
   - `BM_BTreeMapIterate`: Iterate through all words in btree_map

4. **Random Access Performance**
   - `BM_RadixTreeRandomAccess`: Random access patterns
   - `BM_BTreeMapRandomAccess`: Random access patterns in btree_map

5. **Prefix Search Performance**
   - `BM_RadixTreePrefixSearch`: Lookup all words starting with a given prefix in radix tree
   - `BM_BTreeMapPrefixSearch`: Lookup all words starting with a given prefix in btree_map

### Benchmarking Results

```bash
Running ./benchmark
Run on (10 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x10)
Load Average: 2.38, 2.61, 2.57
-----------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------
BM_RadixTreeInsert         64874683 ns     64857000 ns           10 MemoryPeak=16.5591G bytes_per_second=166.489Mi/s items_per_second=3.63702M/s
BM_BTreeMapInsert          14735861 ns     14734067 ns           45 MemoryPeak=369.099M bytes_per_second=732.86Mi/s items_per_second=16.0096M/s
BM_RadixTreeLookup         57541601 ns     57520250 ns           12 MemoryPeak=0 bytes_per_second=93.8626Mi/s items_per_second=4.10092M/s
BM_BTreeMapLookup          17347551 ns     17335244 ns           41 MemoryPeak=0 bytes_per_second=311.447Mi/s items_per_second=13.6073M/s
BM_RadixTreeIterate          746164 ns       746063 ns          924 MemoryPeak=0 bytes_per_second=14.1341Gi/s items_per_second=316.174M/s
BM_BTreeMapIterate           316191 ns       316167 ns         2200 MemoryPeak=0 bytes_per_second=33.3524Gi/s items_per_second=746.081M/s
BM_RadixTreeRandomAccess     618687 ns       618634 ns          998 MemoryPeak=0 bytes_per_second=36.9979Mi/s items_per_second=1.61646M/s
BM_BTreeMapRandomAccess      188974 ns       188905 ns         3748 MemoryPeak=0 bytes_per_second=121.162Mi/s items_per_second=5.29367M/s
BM_RadixTreePrefixSearch      94661 ns        94576 ns         7382 MemoryPeak=0 bytes_per_second=10.7576Gi/s items_per_second=240.643M/s
BM_BTreeMapPrefixSearch       52531 ns        52503 ns        12272 MemoryPeak=0 bytes_per_second=19.3782Gi/s items_per_second=433.484M/s
```

## Dependencies

- C++17 compiler
- Google Benchmark (for benchmarks)
- Abseil (for btree_map comparison)

## License

This project is open source. See the original Go implementation for licensing details.
