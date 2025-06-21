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

### Benchmarking Results

```bash
Loaded 235886 words
Radix tree size: 235886
BTree map size: 235886
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2025-06-21T09:20:01+05:30
Running ./benchmark
Run on (14 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x14)
Load Average: 1.89, 2.25, 2.81
---------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------
BM_RadixTreeInsert            420537125 ns    420417000 ns            2 bytes_per_second=25.684Mi/s items_per_second=561.076k/s
BM_BTreeMapInsert              14744467 ns     14743152 ns           46 bytes_per_second=732.408Mi/s items_per_second=15.9997M/s
BM_RadixTreeLookup             32559549 ns     32555227 ns           22 bytes_per_second=165.841Mi/s items_per_second=7.24572M/s
BM_BTreeMapLookup              19712880 ns     19711086 ns           35 bytes_per_second=273.907Mi/s items_per_second=11.9672M/s
BM_RadixTreeIterate                1046 ns         1046 ns       681292 bytes_per_second=9.84546Ti/s items_per_second=225.525G/s
BM_BTreeMapIterate               343588 ns       343541 ns         2156 bytes_per_second=30.6948Gi/s items_per_second=686.632M/s
BM_RadixTreeRandomAccess         428159 ns       428112 ns         1365 bytes_per_second=53.4631Mi/s items_per_second=2.33584M/s
BM_BTreeMapRandomAccess          166930 ns       166912 ns         4056 bytes_per_second=137.127Mi/s items_per_second=5.99116M/s
```

```bash
Generated 100000 UUIDs
Radix tree size: 99998
BTree map size: 99998
Sample UUIDs:
  01cae807-ca8e-ee8d-809f-1a3808d65ad9
  a120964f-18d4-398d-f89c-2d2cbf0e73d4
  ecd282f7-e513-b7a7-85cb-b43eed11a91c
  9c0d7398-12bc-4d51-b7f3-abedad89918a
  42d50d47-2f2a-2ee3-b11c-22a662bd025b
Unable to determine clock rate from sysctl: hw.cpufrequency: No such file or directory
This does not affect benchmark measurements, only the metadata output.
***WARNING*** Failed to set thread affinity. Estimated CPU frequency may be incorrect.
2025-06-21T11:15:19+05:30
Running ./benchmark-uuid
Run on (14 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x14)
Load Average: 3.69, 3.52, 3.73
---------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------
BM_RadixTreeUUIDIterate             504 ns          503 ns      1000000 bytes_per_second=8.67637Ti/s items_per_second=198.745G/s
BM_BTreeMapUUIDIterate           322856 ns       322837 ns         2136 bytes_per_second=13.8471Gi/s items_per_second=309.754M/s
BM_RadixTreeUUIDLookup         18683813 ns     18681771 ns           35 bytes_per_second=122.516Mi/s items_per_second=5.35281M/s
BM_BTreeMapUUIDLookup          18586938 ns     18584865 ns           37 bytes_per_second=123.155Mi/s items_per_second=5.38072M/s
BM_RadixTreeUUIDRandomAccess     539809 ns       539733 ns         1283 bytes_per_second=42.4065Mi/s items_per_second=1.85277M/s
BM_BTreeMapUUIDRandomAccess      262353 ns       262317 ns         2827 bytes_per_second=87.2538Mi/s items_per_second=3.81218M/s
```

### Key Performance Insights

- **Iteration**: Radix tree is ~300x faster than btree_map due to leaf-based iteration
- **Insertion**: BTree map is faster due to simpler node structure
- **Lookup**: BTree map is slightly faster for random access
- **Memory**: Radix tree uses prefix compression for better memory efficiency

### Performance Tips

1. **Run in release mode**: Always use `-O3` optimization
2. **Multiple runs**: Use `--benchmark_repetitions=3` for reliable results
3. **Performance counters**: Enable hardware counters for detailed analysis

## Dependencies

- C++17 compiler
- Google Benchmark (for benchmarks)
- Abseil (for btree_map comparison)

## License

This project is open source. See the original Go implementation for licensing details.
