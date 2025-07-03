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
Run on (14 X 24 MHz CPU s)
CPU Caches:
  L1 Data 64 KiB
  L1 Instruction 128 KiB
  L2 Unified 4096 KiB (x14)
Load Average: 3.36, 3.77, 3.94
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations MemoryPeak MemoryPerItem PeakPerItem   RSSDelta TotalMemory bytes_per_second items_per_second
-----------------------------------------------------------------------------------------------------------------------------------------------------------------
BM_RadixTreeInsert         71118185 ns     71009333 ns            9   117.306G        497.3k      497.3k          0    117.306G      152.065Mi/s        3.3219M/s
BM_BTreeMapInsert          15959900 ns     15801778 ns           45   137.875G      584.499k    584.499k          0    137.875G      683.341Mi/s       14.9278M/s
---------------------------------------------------------------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations MemoryDelta MemoryPeak   RSSDelta bytes_per_second items_per_second
---------------------------------------------------------------------------------------------------------------------------------------
BM_RadixTreeLookup         58941472 ns     58919917 ns           12           0   137.875G          0      91.6329Mi/s        4.0035M/s
BM_BTreeMapLookup          17426093 ns     17424475 ns           40           0   137.976G          0      309.852Mi/s       13.5376M/s
----------------------------------------------------------------------------------------------------------------
Benchmark                         Time             CPU   Iterations MemoryPeak bytes_per_second items_per_second
----------------------------------------------------------------------------------------------------------------
BM_RadixTreeIterate          913956 ns       912172 ns          752          0      11.5602Gi/s       258.598M/s
BM_BTreeMapIterate           325366 ns       325335 ns         2143          0      32.4126Gi/s       725.057M/s
BM_RadixTreeRandomAccess     545712 ns       545420 ns         1177          0      41.9643Mi/s       1.83345M/s
BM_BTreeMapRandomAccess      178879 ns       178839 ns         3855          0      127.982Mi/s       5.59162M/s
BM_RadixTreePrefixSearch      95467 ns        95460 ns         7277          0      10.6579Gi/s       238.414M/s
BM_BTreeMapPrefixSearch       54735 ns        54720 ns        13142          0      18.5929Gi/s       415.917M/s
```

## Dependencies

- C++17 compiler
- Google Benchmark (for benchmarks)
- Abseil (for btree_map comparison)

## License

This project is open source. See the original Go implementation for licensing details.

## Benchmark Memory Usage Metrics

When you run `./run_benchmark.sh`, the output includes several memory usage metrics for each benchmark:

- **TotalMemory**: The total memory used by the process after all insertions. This is the main metric for total memory consumed by your data structure.
- **MemoryPeak**: The peak memory usage observed during the benchmark.
- **RSSDelta**: The change in Resident Set Size (physical memory used) during the benchmark.
- **MemoryPerItem**: The average memory usage per item (for insert benchmarks).
- **PeakPerItem**: The peak memory usage per item (for insert benchmarks).

### Example Output

```
Benchmark           ...   TotalMemory   MemoryPeak   MemoryPerItem   PeakPerItem   RSSDelta
BM_RadixTreeInsert  ...   105.764G      105.764G     448.367k        448.367k      0
BM_BTreeMapInsert   ...   126.987G      126.987G     538.339k        538.339k      0
```

- **TotalMemory** is the value you should use to compare the total memory consumed by different data structures after all insertions.

### JSON Output

For more detailed analysis, you can run:

```sh
./benchmark --benchmark_format=json > results.json
```

This will output all memory statistics in JSON format for further processing or visualization.
