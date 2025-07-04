#include <iostream>
#include <string>
#include <vector>
#include "radix/tree.hpp"

int main() {
    std::cout << "Testing findMatchingPrefixes basic functionality..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Insert test data
    tree.insert("a", "alpha");
    tree.insert("ab", "beta");
    tree.insert("abc", "gamma");
    tree.insert("b", "bravo");
    tree.insert("c", "charlie");
    
    std::cout << "Tree size: " << tree.len() << std::endl;
    
    // Test findMatchingPrefixes
    std::cout << "\nTesting findMatchingPrefixes for 'abc':" << std::endl;
    auto results = tree.findMatchingPrefixes("abc");
    
    std::cout << "Found " << results.size() << " matching prefixes:" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    
    // Test with non-existent key
    std::cout << "\nTesting findMatchingPrefixes for 'abb':" << std::endl;
    results = tree.findMatchingPrefixes("abb");
    
    std::cout << "Found " << results.size() << " matching prefixes:" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    
    std::cout << "Test completed!" << std::endl;
    return 0;
} 