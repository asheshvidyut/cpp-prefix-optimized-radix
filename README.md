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
BM_RadixTreeInsert         71248346 ns     71037500 ns           10 MemoryPeak=1.1073G bytes_per_second=152.004Mi/s items_per_second=3.32058M/s
BM_BTreeMapInsert          17749368 ns     16790405 ns           42 MemoryPeak=2.38236G bytes_per_second=643.106Mi/s items_per_second=14.0489M/s
BM_RadixTreeLookup         63523439 ns     63523455 ns           11 MemoryPeak=0 bytes_per_second=84.9923Mi/s items_per_second=3.71337M/s
BM_BTreeMapLookup          17925415 ns     17925410 ns           39 MemoryPeak=0 bytes_per_second=301.193Mi/s items_per_second=13.1593M/s
BM_RadixTreeIterate          664484 ns       664486 ns         1013 MemoryPeak=0 bytes_per_second=15.8693Gi/s items_per_second=354.99M/s
BM_BTreeMapIterate           300113 ns       300112 ns         2332 MemoryPeak=0 bytes_per_second=35.1366Gi/s items_per_second=785.992M/s
BM_RadixTreeRandomAccess     528395 ns       528394 ns         1199 MemoryPeak=0 bytes_per_second=43.3165Mi/s items_per_second=1.89253M/s
BM_BTreeMapRandomAccess      170838 ns       170838 ns         4055 MemoryPeak=0 bytes_per_second=133.976Mi/s items_per_second=5.85348M/s
BM_RadixTreeSeekPrefix        92119 ns        92118 ns         7606 MemoryPeak=0 bytes_per_second=11.0447Gi/s items_per_second=247.065M/s
BM_BTreeMapPrefixSearch     1005711 ns      1005711 ns          693 MemoryPeak=0 bytes_per_second=1.01163Gi/s items_per_second=22.6298M/s
```

**Key insight:**
- `BM_RadixTreeSeekPrefix` is over **10x faster** than `BM_BTreeMapPrefixSearch` for finding all words starting with 's'.
- This demonstrates the efficiency of the radix tree for prefix queries compared to general-purpose ordered maps.
- For workloads with heavy prefix search requirements, the radix tree offers a significant performance advantage.

## Key Insights from Benchmarks

- **Insertion:**
  - BTreeMap is significantly faster than Radix Tree for bulk insertions (see BM_BTreeMapInsert vs BM_RadixTreeInsert).
  - BTreeMap also uses more peak memory during insertion, while Radix Tree is more memory efficient for this phase.

- **Lookup:**
  - BTreeMap provides much faster point lookups than Radix Tree (BM_BTreeMapLookup vs BM_RadixTreeLookup).
  - For workloads dominated by exact key lookups, BTreeMap is preferable.

- **Iteration:**
  - Both data structures are extremely fast for full iteration, but BTreeMap is about 2x faster (BM_BTreeMapIterate vs BM_RadixTreeIterate).

- **Random Access:**
  - BTreeMap is about 3x faster for random access patterns (BM_BTreeMapRandomAccess vs BM_RadixTreeRandomAccess).

- **Prefix Search:**
  - Radix Tree is over **10x faster** than BTreeMap for prefix queries (BM_RadixTreeSeekPrefix vs BM_BTreeMapPrefixSearch).
  - This is the main advantage of the Radix Tree: if your workload involves many prefix or range queries, it will outperform general-purpose ordered maps.

**Summary:**
- Use **Radix Tree** for workloads with heavy prefix search or range query requirements.
- Use **BTreeMap** for workloads dominated by insertions, lookups, or random access.
- Both structures are highly efficient for iteration, but BTreeMap is generally faster for most operations except prefix search.

## Dependencies

- C++17 compiler
- Google Benchmark (for benchmarks)
- Abseil (for btree_map comparison)

## License

This project is open source. See the original Go implementation for licensing details.
