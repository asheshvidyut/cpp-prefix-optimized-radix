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

#### Ubuntu/Debian:
```bash
sudo apt-get install libbenchmark-dev libabsl-dev
```

### Build
```bash
make all
```

### Run Tests
```bash
./fuzzy-test
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
│   └── fuzzy_test.cpp # Fuzzy testing
├── main.cpp          # Example usage
├── benchmark.cpp     # Performance benchmarks
├── fuzzy_test_main.cpp # Fuzzy test runner
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

4. **Cache Performance**
   - `BM_RadixTreeRandomAccess`: Random access patterns
   - `BM_BTreeMapRandomAccess`: Random access patterns in btree_map
   - `BM_RadixTreeSequentialAccess`: Sequential access patterns
   - `BM_BTreeMapSequentialAccess`: Sequential access patterns in btree_map

### Benchmarking Results

```
Loaded 235886 words
Radix tree size: 235886
BTree map size: 235886

BM_RadixTreeInsert            431447812 ns    431297500 ns            2
BM_BTreeMapInsert              15949482 ns     15875477 ns           44
BM_RadixTreeLookup             34521169 ns     34521150 ns           20
BM_BTreeMapLookup              19849081 ns     19849056 ns           36
BM_RadixTreeIterate                1085 ns         1085 ns       612327
BM_BTreeMapIterate               317448 ns       317448 ns         2175
BM_RadixTreeRandomAccess         406194 ns       406194 ns         1705
BM_BTreeMapRandomAccess          171449 ns       171295 ns         3904
BM_RadixTreeSequentialAccess   35048865 ns     35048850 ns           20
BM_BTreeMapSequentialAccess    19788282 ns     19788029 ns           35
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
