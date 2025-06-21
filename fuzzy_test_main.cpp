//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "radix/fuzzy_test.cpp"
#include <iostream>
#include <chrono>

int main() {
    std::cout << "Starting fuzzy tests for immutable radix tree..." << std::endl;
    
    // Record start time
    auto start = std::chrono::high_resolution_clock::now();
    
    // Run all fuzzy tests
    runFuzzyTests();
    
    // Record end time
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate duration
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "All fuzzy tests completed in " << duration.count() << " milliseconds." << std::endl;
    
    return 0;
} 