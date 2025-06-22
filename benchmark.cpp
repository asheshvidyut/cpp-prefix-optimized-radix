#include <benchmark/benchmark.h>
#include <absl/container/btree_map.h>
#include <absl/strings/string_view.h>
#include <fstream>
#include <vector>
#include <string>
#include <random>
#include <iostream>
#include "radix/tree.hpp"

// Global data for benchmarks
std::vector<std::string> words;
Tree<std::string, std::string> radix_tree;
absl::btree_map<std::string, std::string> btree_map;

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

// Benchmark: Insert all words into radix tree
static void BM_RadixTreeInsert(benchmark::State& state) {
    for (auto _ : state) {
        Tree<std::string, std::string> tree;
        for (const auto& word : words) {
            auto [new_tree, old_val, updated] = tree.insert(word, word);
            benchmark::DoNotOptimize(old_val);
            benchmark::DoNotOptimize(updated);
        }
        benchmark::DoNotOptimize(tree);
    }
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_RadixTreeInsert);

// Benchmark: Insert all words into btree_map
static void BM_BTreeMapInsert(benchmark::State& state) {
    for (auto _ : state) {
        absl::btree_map<std::string, std::string> map;
        for (const auto& word : words) {
            auto [it, inserted] = map.insert({word, word});
            benchmark::DoNotOptimize(it);
            benchmark::DoNotOptimize(inserted);
        }
        benchmark::DoNotOptimize(map);
    }
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_BTreeMapInsert);

// Benchmark: Lookup all words in radix tree
static void BM_RadixTreeLookup(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& word : words) {
            auto result = radix_tree.Get(word);
            benchmark::DoNotOptimize(result);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string));
}
BENCHMARK(BM_RadixTreeLookup);

// Benchmark: Lookup all words in btree_map
static void BM_BTreeMapLookup(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& word : words) {
            auto it = btree_map.find(word);
            benchmark::DoNotOptimize(it);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string));
}
BENCHMARK(BM_BTreeMapLookup);

// Benchmark: Iterate through all words in radix tree
static void BM_RadixTreeIterate(benchmark::State& state) {
    std::cout << radix_tree.GetLeavesInSubtree() << std::endl;
    for (auto _ : state) {
        auto iterator = radix_tree.iterator();
        int count = 0;
        while (true) {
            auto result = iterator.next();
            if (!result.found) break;
            benchmark::DoNotOptimize(result.key);
            benchmark::DoNotOptimize(result.val);
            count++;
        }
        benchmark::DoNotOptimize(count);
    }
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_RadixTreeIterate);

// Benchmark: Iterate through all words in btree_map
static void BM_BTreeMapIterate(benchmark::State& state) {
    for (auto _ : state) {
        int count = 0;
        for (const auto& [key, value] : btree_map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(value);
            count++;
        }
        benchmark::DoNotOptimize(count);
    }
    
    state.SetItemsProcessed(state.iterations() * words.size());
    state.SetBytesProcessed(state.iterations() * words.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_BTreeMapIterate);

// Benchmark: Random access patterns (cache performance)
static void BM_RadixTreeRandomAccess(benchmark::State& state) {
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
    
    state.SetItemsProcessed(state.iterations() * 1000);
    state.SetBytesProcessed(state.iterations() * 1000 * sizeof(std::string));
}
BENCHMARK(BM_RadixTreeRandomAccess);

// Benchmark: Random access patterns in btree_map
static void BM_BTreeMapRandomAccess(benchmark::State& state) {
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
    
    state.SetItemsProcessed(state.iterations() * 1000);
    state.SetBytesProcessed(state.iterations() * 1000 * sizeof(std::string));
}
BENCHMARK(BM_BTreeMapRandomAccess);

int main(int argc, char** argv) {
    // Initialize data before running benchmarks
    InitializeData(radix_tree);
    
    // Print some statistics
    std::cout << "Loaded " << words.size() << " words\n";
    std::cout << "Radix tree size: " << radix_tree.len() << "\n";
    std::cout << "BTree map size: " << btree_map.size() << "\n";
    
    // Run benchmarks
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    
    return 0;
} 