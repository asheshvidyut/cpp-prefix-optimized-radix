#include <benchmark/benchmark.h>
#include <absl/container/btree_map.h>
#include <absl/strings/string_view.h>
#include <vector>
#include <string>
#include <random>
#include <iostream>
#include <iomanip>
#include <sstream>
#include "radix/tree.hpp"

// Global data for benchmarks
std::vector<std::string> uuids;
Tree<std::string, std::string> radix_tree;
absl::btree_map<std::string, std::string> btree_map;

// Generate a random UUID
std::string GenerateUUID() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    std::stringstream ss;
    ss << std::hex << std::setfill('0');
    
    // Generate 8-4-4-4-12 format
    for (int i = 0; i < 8; ++i) ss << std::setw(1) << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << std::setw(1) << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << std::setw(1) << dis(gen);
    ss << "-";
    for (int i = 0; i < 4; ++i) ss << std::setw(1) << dis(gen);
    ss << "-";
    for (int i = 0; i < 12; ++i) ss << std::setw(1) << dis(gen);
    
    return ss.str();
}

// Generate random UUIDs
void GenerateUUIDs(int count) {
    uuids.clear();
    uuids.reserve(count);
    
    for (int i = 0; i < count; ++i) {
        uuids.push_back(GenerateUUID());
    }
}

// Initialize data structures with UUIDs
void InitializeUUIDData(int count = 100000) {
    GenerateUUIDs(count);
    
    // Insert all UUIDs into both data structures
    for (const auto& uuid : uuids) {
        radix_tree.insert(uuid, uuid);
        btree_map[uuid] = uuid;
    }
}

// Benchmark: Iterate through all UUIDs in radix tree
static void BM_RadixTreeUUIDIterate(benchmark::State& state) {
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
    
    state.SetItemsProcessed(state.iterations() * uuids.size());
    state.SetBytesProcessed(state.iterations() * uuids.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_RadixTreeUUIDIterate);

// Benchmark: Iterate through all UUIDs in btree_map
static void BM_BTreeMapUUIDIterate(benchmark::State& state) {
    for (auto _ : state) {
        int count = 0;
        for (const auto& [key, value] : btree_map) {
            benchmark::DoNotOptimize(key);
            benchmark::DoNotOptimize(value);
            count++;
        }
        benchmark::DoNotOptimize(count);
    }
    
    state.SetItemsProcessed(state.iterations() * uuids.size());
    state.SetBytesProcessed(state.iterations() * uuids.size() * sizeof(std::string) * 2);
}
BENCHMARK(BM_BTreeMapUUIDIterate);

// Benchmark: Lookup random UUIDs in radix tree
static void BM_RadixTreeUUIDLookup(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& uuid : uuids) {
            auto result = radix_tree.Get(uuid);
            benchmark::DoNotOptimize(result);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * uuids.size());
    state.SetBytesProcessed(state.iterations() * uuids.size() * sizeof(std::string));
}
BENCHMARK(BM_RadixTreeUUIDLookup);

// Benchmark: Lookup random UUIDs in btree_map
static void BM_BTreeMapUUIDLookup(benchmark::State& state) {
    for (auto _ : state) {
        for (const auto& uuid : uuids) {
            auto it = btree_map.find(uuid);
            benchmark::DoNotOptimize(it);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * uuids.size());
    state.SetBytesProcessed(state.iterations() * uuids.size() * sizeof(std::string));
}
BENCHMARK(BM_BTreeMapUUIDLookup);

// Benchmark: Random access patterns with UUIDs
static void BM_RadixTreeUUIDRandomAccess(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, uuids.size() - 1);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            const auto& uuid = uuids[dis(gen)];
            auto result = radix_tree.Get(uuid);
            benchmark::DoNotOptimize(result);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * 1000);
    state.SetBytesProcessed(state.iterations() * 1000 * sizeof(std::string));
}
BENCHMARK(BM_RadixTreeUUIDRandomAccess);

// Benchmark: Random access patterns with UUIDs in btree_map
static void BM_BTreeMapUUIDRandomAccess(benchmark::State& state) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, uuids.size() - 1);
    
    for (auto _ : state) {
        for (int i = 0; i < 1000; ++i) {
            const auto& uuid = uuids[dis(gen)];
            auto it = btree_map.find(uuid);
            benchmark::DoNotOptimize(it);
        }
    }
    
    state.SetItemsProcessed(state.iterations() * 1000);
    state.SetBytesProcessed(state.iterations() * 1000 * sizeof(std::string));
}
BENCHMARK(BM_BTreeMapUUIDRandomAccess);

int main(int argc, char** argv) {
    // Parse command line arguments for UUID count
    int uuid_count = 100000; // Default 100k UUIDs
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--uuid-count" && i + 1 < argc) {
            uuid_count = std::stoi(argv[i + 1]);
            break;
        }
    }
    
    // Initialize data before running benchmarks
    InitializeUUIDData(uuid_count);
    
    // Print some statistics
    std::cout << "Generated " << uuids.size() << " UUIDs\n";
    std::cout << "Radix tree size: " << radix_tree.len() << "\n";
    std::cout << "BTree map size: " << btree_map.size() << "\n";
    
    // Show a few sample UUIDs
    std::cout << "Sample UUIDs:\n";
    for (int i = 0; i < std::min(5, (int)uuids.size()); ++i) {
        std::cout << "  " << uuids[i] << "\n";
    }
    
    // Run benchmarks
    benchmark::Initialize(&argc, argv);
    benchmark::RunSpecifiedBenchmarks();
    
    return 0;
} 