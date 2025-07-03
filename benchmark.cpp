#include <benchmark/benchmark.h>
#include <absl/container/btree_map.h>
#include <absl/strings/string_view.h>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <iostream>
#include <sys/resource.h>
#include "radix/tree.hpp"

// Forward declarations
size_t GetCurrentMemoryUsage();
size_t GetPeakMemoryUsage();
size_t GetRSS();

// Global data for benchmarks
std::vector<std::string> words;
Tree<std::string, std::string> radix_tree;
absl::btree_map<std::string, std::string> btree_map;

// Custom memory manager for detailed tracking
class CustomMemoryManager : public benchmark::MemoryManager {
public:
    void Start() override {
        start_memory_ = GetCurrentMemoryUsage();
        start_rss_ = GetRSS();
    }
    
    void Stop(Result& result) override {
        end_memory_ = GetCurrentMemoryUsage();
        end_rss_ = GetRSS();
        peak_memory_ = GetPeakMemoryUsage();
        
        result.num_allocs = 0;  // We don't track individual allocations
        result.max_bytes_used = peak_memory_;
        result.total_allocated_bytes = end_memory_ - start_memory_;
        result.net_heap_growth = end_memory_ - start_memory_;
    }
    
private:
    size_t start_memory_ = 0;
    size_t end_memory_ = 0;
    size_t start_rss_ = 0;
    size_t end_rss_ = 0;
    size_t peak_memory_ = 0;
};

// Global memory manager instance
static CustomMemoryManager memory_manager;

// Load words from file
void LoadWords() {
    std::ifstream file("words.txt");
    std::string word;
    while (std::getline(file, word)) {
        if (!word.empty()) {
            words.push_back(word);
        }
    }
}

// Initialize data structures
void InitializeData(Tree<std::string, std::string>& radix_tree) {
    std::ifstream file("words.txt");
    std::string word;
    int count = 0;
    
    while (std::getline(file, word)) {
        if (!word.empty()) {
            words.push_back(word);
            auto [new_tree, old_val, updated] = radix_tree.insert(word, word);
            radix_tree = new_tree;
            btree_map[word] = word;
            count++;
            if (count % 10000 == 0) {
                std::cout << "Inserted " << count << " words..." << std::endl;
            }
        }
    }
    
    std::cout << "Total words inserted: " << count << std::endl;
}

// Helper function to get current memory usage
size_t GetCurrentMemoryUsage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes
    }
    return 0;
}

// Helper function to get peak memory usage
size_t GetPeakMemoryUsage() {
    struct rusage usage;
    if (getrusage(RUSAGE_SELF, &usage) == 0) {
        return usage.ru_maxrss * 1024; // Convert KB to bytes
    }
    return 0;
}

// Helper function to get current resident set size
size_t GetRSS() {
    std::ifstream statm("/proc/self/statm");
    if (statm.is_open()) {
        size_t pages;
        statm >> pages;
        return pages * 4096; // Convert pages to bytes (assuming 4KB pages)
    }
    return 0;
}

// Benchmark: Insert all words into radix tree
static void BM_RadixTreeInsert(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    size_t start_rss = GetRSS();
    
    for (auto _ : state) {
        // Clear the existing tree instead of creating a new one
        radix_tree = Tree<std::string, std::string>();
        
        for (const auto& word : words) {
            auto [new_tree, old_val, updated] = radix_tree.insert(word, word);
            radix_tree = new_tree;
            benchmark::DoNotOptimize(old_val);
            benchmark::DoNotOptimize(updated);
        }
        benchmark::DoNotOptimize(radix_tree);
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    size_t end_rss = GetRSS();
    size_t peak_memory = GetPeakMemoryUsage();
    
    // Memory counters
    state.counters["TotalMemory"] = end_memory;
    state.counters["MemoryPeak"] = peak_memory;
    state.counters["RSSDelta"] = end_rss - start_rss;
    state.counters["MemoryPerItem"] = end_memory / words.size();
    state.counters["PeakPerItem"] = peak_memory / words.size();
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_RadixTreeInsert);

// Benchmark: Insert all words into btree_map
static void BM_BTreeMapInsert(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    size_t start_rss = GetRSS();
    
    for (auto _ : state) {
        // Clear the existing map instead of creating a new one
        btree_map.clear();
        
        for (const auto& word : words) {
            auto [it, inserted] = btree_map.insert({word, word});
            benchmark::DoNotOptimize(it);
            benchmark::DoNotOptimize(inserted);
        }
        benchmark::DoNotOptimize(btree_map);
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    size_t end_rss = GetRSS();
    size_t peak_memory = GetPeakMemoryUsage();
    
    // Memory counters
    state.counters["TotalMemory"] = end_memory;
    state.counters["MemoryPeak"] = peak_memory;
    state.counters["RSSDelta"] = end_rss - start_rss;
    state.counters["MemoryPerItem"] = end_memory / words.size();
    state.counters["PeakPerItem"] = peak_memory / words.size();
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_BTreeMapInsert);

// Benchmark: Lookup all words in radix tree
static void BM_RadixTreeLookup(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    size_t start_rss = GetRSS();
    
    for (auto _ : state) {
        for (const auto& word : words) {
            auto result = radix_tree.Get(word);
            benchmark::DoNotOptimize(result);
        }
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    size_t end_rss = GetRSS();
    size_t peak_memory = GetPeakMemoryUsage();
    
    // Memory counters
    state.counters["MemoryDelta"] = end_memory - start_memory;
    state.counters["MemoryPeak"] = peak_memory;
    state.counters["RSSDelta"] = end_rss - start_rss;
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string));
}
BENCHMARK(BM_RadixTreeLookup);

// Benchmark: Lookup all words in btree_map
static void BM_BTreeMapLookup(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    size_t start_rss = GetRSS();
    
    for (auto _ : state) {
        for (const auto& word : words) {
            auto it = btree_map.find(word);
            benchmark::DoNotOptimize(it);
        }
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    size_t end_rss = GetRSS();
    size_t peak_memory = GetPeakMemoryUsage();
    
    // Memory counters
    state.counters["MemoryDelta"] = end_memory - start_memory;
    state.counters["MemoryPeak"] = peak_memory;
    state.counters["RSSDelta"] = end_rss - start_rss;
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string));
}
BENCHMARK(BM_BTreeMapLookup);

// Benchmark: Iterate through all words in radix tree
static void BM_RadixTreeIterate(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    
    for (auto _ : state) {
        int count = 0;
        // Use range-based for loop
        for (const auto& [key, value] : radix_tree) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(value);
            count++;
        }
        benchmark::DoNotOptimize(count);
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    state.counters["MemoryPeak"] = end_memory - start_memory;
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_RadixTreeIterate);

// Benchmark: Iterate through all words in btree_map
static void BM_BTreeMapIterate(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    
    for (auto _ : state) {
        int count = 0;
        for (const auto& [key, value] : btree_map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(value);
            count++;
        }
        benchmark::DoNotOptimize(count);
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    state.counters["MemoryPeak"] = end_memory - start_memory;
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_BTreeMapIterate);

// Benchmark: Random access patterns (cache performance)
static void BM_RadixTreeRandomAccess(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, words.size() - 1);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            const auto& word = words[dis(gen)];
            auto result = radix_tree.Get(word);
            benchmark::DoNotOptimize(result);
        }
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    state.counters["MemoryPeak"] = end_memory - start_memory;
    state.SetItemsProcessed(state.iterations() * 1000);
    state.SetBytesProcessed(state.iterations() * 1000 * sizeof(std::string));
}
BENCHMARK(BM_RadixTreeRandomAccess);

// Benchmark: Random access patterns in btree_map
static void BM_BTreeMapRandomAccess(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, words.size() - 1);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            const auto& word = words[dis(gen)];
            auto it = btree_map.find(word);
            benchmark::DoNotOptimize(it);
        }
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    state.counters["MemoryPeak"] = end_memory - start_memory;
    state.SetItemsProcessed(state.iterations() * 1000);
    state.SetBytesProcessed(state.iterations() * 1000 * sizeof(std::string));
}
BENCHMARK(BM_BTreeMapRandomAccess);

// Benchmark: Prefix search using radix tree seekPrefix("s")
static void BM_RadixTreePrefixSearch(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    
    for (auto _ : state) {
        int count = 0;
        auto iterator = radix_tree.iterator();
        iterator.seekPrefix("s");
        
        while (true) {
            auto result = iterator.next();
            if (!result.found) break;
            
            if (result.key.find("s") == 0) {
                benchmark::DoNotOptimize(result.key);
                benchmark::DoNotOptimize(result.val);
                count++;
            }
        }
        benchmark::DoNotOptimize(count);
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    state.counters["MemoryPeak"] = end_memory - start_memory;
    state.SetItemsProcessed(state.iterations() * 22759); // Approximate count of words starting with 's'
    state.SetBytesProcessed(state.iterations() * 22759 * sizeof(std::string) * 2);
}
BENCHMARK(BM_RadixTreePrefixSearch);

// Benchmark: Prefix search using BTree lower_bound and upper_bound
static void BM_BTreeMapPrefixSearch(benchmark::State& state) {
    size_t start_memory = GetCurrentMemoryUsage();
    
    for (auto _ : state) {
        int count = 0;
        
        // Use lower_bound to find the first key >= "s"
        auto lower = btree_map.lower_bound("s");
        
        // Use upper_bound to find the first key > "s" + max char
        // For prefix "s", we want all keys that start with "s"
        // The next prefix after "s" would be "t", so we use "t" as upper bound
        auto upper = btree_map.upper_bound("t");
        
        // Iterate from lower_bound to upper_bound
        for (auto it = lower; it != upper; ++it) {
            const auto& [key, value] = *it;
            // Double-check that the key actually starts with "s" (should be redundant but safe)
            if (key.find("s") == 0) {
                benchmark::DoNotOptimize(key);
                benchmark::DoNotOptimize(value);
                count++;
            }
        }
        benchmark::DoNotOptimize(count);
    }
    
    size_t end_memory = GetCurrentMemoryUsage();
    state.counters["MemoryPeak"] = end_memory - start_memory;
    state.SetItemsProcessed(state.iterations() * 22759); // Approximate count of words starting with 's'
    state.SetBytesProcessed(state.iterations() * 22759 * sizeof(std::string) * 2);
}
BENCHMARK(BM_BTreeMapPrefixSearch);

int main(int argc, char** argv) {
    // Initialize data before running benchmarks
    InitializeData(radix_tree);
    
    // Print some statistics
    std::cout << "Loaded " << words.size() << " words\n";
    std::cout << "Radix tree size: " << radix_tree.len() << "\n";
    std::cout << "BTree map size: " << btree_map.size() << "\n";
    
    // Register memory manager (commented out due to API issues)
    // benchmark::MemoryManager::Register(&memory_manager);
    
    // Run benchmarks
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    
    return 0;
} 