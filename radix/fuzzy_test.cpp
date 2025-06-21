//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "tree.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <random>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <iomanip>
#include <sstream>
#include <map>
#include "iterator.cpp"

// Helper function to generate a random string from a limited alphabet
std::string generateReadableString(size_t length) {
    static const char alphabet[] = "abcdefg";
    static std::random_device rd;
    static std::mt19937 gen(rd());
    static std::uniform_int_distribution<> dis(0, sizeof(alphabet) - 2);
    
    std::string result;
    result.reserve(length);
    for (size_t i = 0; i < length; ++i) {
        result += alphabet[dis(gen)];
    }
    return result;
}

// Test function to verify that iterating in the radix tree
// produces the same results as filtering a sorted list of keys
void testIterateFuzz() {
    Tree<std::string, int> tree;
    std::vector<std::string> keys;
    std::map<std::string, int> expectedValues;
    
    // Generate random keys and values
    for (int i = 0; i < 1000; ++i) {
        std::string key = generateReadableString(5);
        int value = i;
        keys.push_back(key);
        expectedValues[key] = value;
        
        auto [newTree, oldVal, didUpdate] = tree.insert(key, value);
        tree = newTree;
    }
    
    // Sort keys for comparison
    std::sort(keys.begin(), keys.end());
    
    // Test iteration from random points
    std::vector<std::pair<std::string, int>> radixResults;
    auto it = tree.iterator();
    it.seekPrefix("");
    IteratorResult<std::string, int> result;
    while ((result = it.next()).found) {
        radixResults.push_back({result.key, result.val});
    }

    
    std::vector<std::pair<std::string, int>> expectedResults;
    // iterate over expectedValues
    int idx = 0;
    for (auto &kv : expectedValues) {
        std::string key = kv.first;
        if (idx < radixResults.size() && radixResults[idx].first == key) {
            idx++;
        } else {
            std::cout << "ERROR: Not sorted" << std::endl;
        }
        expectedResults.push_back({key, expectedValues[key]});
    }

    std::cout << "radixResults: " << radixResults.size() << std::endl;
    std::cout << "expectedResults: " << expectedResults.size() << std::endl;

    
    if (radixResults != expectedResults) {
        std::cerr << "Iteration test failed!" << std::endl;
        exit(1);
    }
}

// Test function to verify that the radix tree behaves correctly with random operations
void testRandomOperationsFuzz() {
    Tree<std::string, int> tree;
    std::map<std::string, int> expectedValues;
    
    // Perform random operations
    for (int i = 0; i < 10000; ++i) {
        std::string key = generateReadableString(5);
        int value = rand() % 1000;
        
        int op = rand() % 3;
        switch (op) {
            case 0: { // Insert
                auto [newTree, oldVal, didUpdate] = tree.insert(key, value);
                tree = newTree;
                expectedValues[key] = value;
                break;
            }
            case 1: { // Delete
                auto [newTree, oldVal, didUpdate] = tree.del(key);
                tree = newTree;
                expectedValues.erase(key);
                break;
            }
            case 2: { // Get
                auto it = expectedValues.find(key);
                if (it != expectedValues.end()) {
                    auto val = tree.Get(key);
                    if (!val || val.value() != it->second) {
                        std::cerr << "Get operation test failed!" << std::endl;
                        exit(1);
                    }
                }
                break;
            }
        }
    }
    
    // Verify final state
    for (const auto& pair : expectedValues) {
        auto val = tree.Get(pair.first);
        if (!val || val.value() != pair.second) {
            std::cerr << "Final state verification failed!" << std::endl;
            exit(1);
        }
    }
}

void runFuzzyTests() {
    std::cout << "Running fuzzy tests..." << std::endl;
    
    testIterateFuzz();
    std::cout << "Iteration test passed" << std::endl;
    
    testRandomOperationsFuzz();
    std::cout << "Random operations test passed" << std::endl;
    
    std::cout << "All fuzzy tests passed!" << std::endl;
} 