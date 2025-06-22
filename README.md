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
Run on (14 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x14)
Load Average: 2.52, 2.86, 3.25
-----------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations UserCounters...
-----------------------------------------------------------------------------------
BM_RadixTreeInsert         68868389 ns     68764778 ns            9 MemoryPeak=16.794G bytes_per_second=157.028Mi/s items_per_second=3.43033M/s
BM_BTreeMapInsert          14789440 ns     14763250 ns           48 MemoryPeak=1.19118G bytes_per_second=731.411Mi/s items_per_second=15.9779M/s
BM_RadixTreeLookup         69642708 ns     63624545 ns           11 MemoryPeak=0 bytes_per_second=84.8572Mi/s items_per_second=3.70747M/s
BM_BTreeMapLookup          20060848 ns     19999513 ns           39 MemoryPeak=0 bytes_per_second=269.957Mi/s items_per_second=11.7946M/s
BM_RadixTreeIterate         1088449 ns      1085696 ns          481 MemoryPeak=0 bytes_per_second=9.71259Gi/s items_per_second=217.267M/s
BM_BTreeMapIterate           316798 ns       316545 ns         2168 MemoryPeak=0 bytes_per_second=33.3126Gi/s items_per_second=745.19M/s
BM_RadixTreeRandomAccess     733874 ns       732446 ns         1171 MemoryPeak=0 bytes_per_second=31.249Mi/s items_per_second=1.36529M/s
BM_BTreeMapRandomAccess      318198 ns       314154 ns         2282 MemoryPeak=0 bytes_per_second=72.8566Mi/s items_per_second=3.18315M/s```
```

### Benchmark UUID

```bash
Running ./benchmark-uuid
Run on (14 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x14)
Load Average: 3.23, 3.09, 3.32
---------------------------------------------------------------------------------------
Benchmark                             Time             CPU   Iterations UserCounters...
---------------------------------------------------------------------------------------
BM_RadixTreeUUIDIterate         9732653 ns      9724241 ns           54 bytes_per_second=470.745Mi/s items_per_second=10.2836M/s
BM_BTreeMapUUIDIterate           313932 ns       313876 ns         2180 bytes_per_second=14.2424Gi/s items_per_second=318.597M/s
BM_RadixTreeUUIDLookup         38121473 ns     38106353 ns           17 bytes_per_second=60.064Mi/s items_per_second=2.62423M/s
BM_BTreeMapUUIDLookup          19803780 ns     19774091 ns           33 bytes_per_second=115.748Mi/s items_per_second=5.05712M/s
BM_RadixTreeUUIDRandomAccess     813450 ns       805547 ns          967 bytes_per_second=28.4132Mi/s items_per_second=1.24139M/s
BM_BTreeMapUUIDRandomAccess      290895 ns       290831 ns         2269 bytes_per_second=78.6993Mi/s items_per_second=3.43843M/s
```

## Dependencies

- C++17 compiler
- Google Benchmark (for benchmarks)
- Abseil (for btree_map comparison)

## License

This project is open source. See the original Go implementation for licensing details.
