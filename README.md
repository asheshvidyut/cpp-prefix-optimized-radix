# CPP Prefix Optimized Radix Tree

A C++ implementation of a prefix-optimized radix tree with leaf-based iteration, inspired by the Go implementation at https://github.com/asheshvidyut/prefix-search-optimized-radix/.

## Features

- **Mutable Operations**: Insert, delete, and update operations modify nodes in place
- **Leaf-Based Iteration**: Efficient iteration using leaf linking for O(1) next/prev operations
- **Prefix Compression**: Shared prefixes are stored once to reduce memory usage
- **Generic Design**: Template-based implementation supporting any key-value types
- **Comprehensive Benchmarking**: Performance comparison with Abseil's btree_map
- **Longest Prefix Search**: Find the longest matching prefix for a given key
- **Index-Based Access**: Retrieve elements by their sorted index position
- **Prefix Matching**: Find all keys that are prefixes of a given key

## Unique APIs (Not Available in btree_map)

### Longest Prefix Search

The radix tree provides a `LongestPrefix` method that finds the longest key that is a prefix of the given search key. This is particularly useful for:

- **IP routing tables**: Finding the most specific route for an IP address
- **URL routing**: Matching the longest path prefix
- **Autocomplete**: Finding the best matching prefix for user input

```cpp
Tree<std::string, std::string> tree;
tree.insert("", "empty");
tree.insert("foo", "value1");
tree.insert("foobar", "value2");

auto result = tree.LongestPrefix("foobaz");
// result.key = "foo", result.val = "value1", result.found = true
```

### Index-Based Access (GetAtIndex)

The `GetAtIndex` method allows you to retrieve elements by their position in the sorted order, which is not available in standard associative containers like `btree_map`:

```cpp
Tree<std::string, std::string> tree;
tree.insert("banana", "fruit2");
tree.insert("apple", "fruit1");
tree.insert("cherry", "fruit3");

// Get the first element in sorted order
auto [key, value, found] = tree.GetAtIndex(0);
// key = "apple", value = "fruit1", found = true

// Get the second element in sorted order
auto [key2, value2, found2] = tree.GetAtIndex(1);
// key2 = "banana", value2 = "fruit2", found2 = true
```

This is useful for:
- **Range queries**: Accessing elements in sorted order by index
- **Pagination**: Implementing page-based access to sorted data
- **Statistical operations**: Finding median, percentiles, etc.



### Find Matching Prefixes

The `findMatchingPrefixes` method finds all keys that are prefixes of a given key:

```cpp
Tree<std::string, std::string> tree;
tree.insert("", "empty");
tree.insert("foo", "value1");
tree.insert("foobar", "value2");
tree.insert("foobaz", "value3");

// Find all prefixes of "foobaz"
auto prefixes = tree.findMatchingPrefixes("foobaz");
for (const auto& [key, value] : prefixes) {
    std::cout << key << ": " << value << std::endl;
}
// Output:
// : empty
// foo: value1
// foobaz: value3
```

This is useful for:
- **Hierarchical data**: Finding all parent nodes in a tree structure
- **URL routing**: Finding all matching route prefixes
- **Configuration systems**: Finding all applicable configuration levels

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
│   ├── iterator.cpp  # Leaf-based iterator and PrefixIterator
├── main.cpp          # Example usage
├── benchmark.cpp     # Performance benchmarks
├── benchmark_uuid.cpp # UUID-based benchmarks
├── test_*.cpp        # Various test files for different features
└── words.txt         # Test data
```

## Testing

The project includes comprehensive tests for all features:

- **Iterator Tests**: Test forward, reverse, and path iteration
- **Find Matching Prefixes Tests**: Test prefix matching functionality
- **Integration Tests**: Test all features working together

## Key Design Decisions

1. **Mutable Operations**: Nodes are modified in place during insert/delete operations
2. **Leaf Linking**: Each node maintains `minLeaf`, `maxLeaf`, and `leaves_in_subtree` properties
3. **Efficient Iteration**: Iterator uses leaf links for O(1) next/prev operations
4. **Memory Optimization**: Prefix compression reduces memory usage for similar keys

## Benchmarking

This project includes comprehensive benchmarks comparing the radix tree with Abseil's `btree_map`.

### Running Benchmarks

```bash
./run_benchmark.sh
```

## Benchmark Memory Usage Metrics

When you run `./run_benchmark.sh`, the output includes several memory usage metrics for each benchmark:

- **TotalMemory**: The total memory used by the process after all insertions. This is the main metric for total memory consumed by your data structure.
- **MemoryPeak**: The peak memory usage observed during the benchmark.
- **RSSDelta**: The change in Resident Set Size (physical memory used) during the benchmark.
- **MemoryPerItem**: The average memory usage per item (for insert benchmarks).
- **PeakPerItem**: The peak memory usage per item (for insert benchmarks).

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
Run on (14 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x14)
Load Average: 3.36, 3.77, 3.94
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations MemoryPeak MemoryPerItem PeakPerItem   RSSDelta TotalMemory bytes_per_second items_per_second
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
BM_RadixTreeInsert         76719819 ns     76662222 ns            9   116.501G      493.886k    493.886k          0    116.501G      140.852Mi/s       3.07695M/s
BM_BTreeMapInsert          15811056 ns     15806116 ns           43   136.969G      580.658k    580.658k          0    136.969G      683.154Mi/s       14.9237M/s
---------------------------------------------------------------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations MemoryDelta MemoryPeak   RSSDelta bytes_per_second items_per_second
---------------------------------------------------------------------------------------------------------------------------------------
BM_RadixTreeLookup         31249167 ns     31246545 ns           22           0   136.969G          0      172.787Mi/s       7.54919M/s
BM_BTreeMapLookup          17625066 ns     17623675 ns           40           0   136.969G          0      306.349Mi/s       13.3846M/s
----------------------------------------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations MemoryPeak bytes_per_second items_per_second
----------------------------------------------------------------------------------------------------------------
BM_RadixTreeIterate          805254 ns       805116 ns          716          0      13.0974Gi/s       292.984M/s
BM_BTreeMapIterate           339719 ns       339687 ns         2087          0      31.0431Gi/s       694.422M/s
BM_RadixTreeRandomAccess     450193 ns       449244 ns         1374          0      50.9482Mi/s       2.22596M/s
BM_BTreeMapRandomAccess      179410 ns       179388 ns         3910          0      127.591Mi/s       5.57452M/s
BM_RadixTreePrefixSearch      96404 ns        96396 ns         7172          0      10.5544Gi/s       236.098M/s
BM_BTreeMapPrefixSearch       54059 ns        54056 ns        12965          0      18.8215Gi/s        421.03M/s
```

## Dependencies

- C++17 compiler
- Google Benchmark (for benchmarks)
- Abseil (for btree_map comparison)

## License

This project is open source. See the original Go implementation for licensing details.
