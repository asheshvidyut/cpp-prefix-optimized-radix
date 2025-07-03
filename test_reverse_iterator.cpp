#include "radix/tree.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

void testReverseIteratorBasic() {
    std::cout << "=== Testing ReverseIterator Basic Functionality ===" << std::endl;
    
    // Test basic reverse iteration
    Tree<std::string, std::string> tree;
    std::vector<std::string> keys = {"apple", "banana", "cherry", "date", "elderberry"};
    
    for (const auto& key : keys) {
        auto [newTree, oldVal, didUpdate] = tree.insert(key, key);
        tree = newTree;
    }
    
    std::cout << "Testing reverse iteration through all keys:" << std::endl;
    auto iter = ReverseIterator<std::string, std::string>(tree.getRoot());
    
    // Should iterate in reverse order: elderberry, date, cherry, banana, apple
    std::vector<std::string> expected = {"elderberry", "date", "cherry", "banana", "apple"};
    
    for (size_t i = 0; i < expected.size(); i++) {
        auto res = iter.previous();
        if (!res.found) {
            std::cout << "ERROR: Expected '" << expected[i] << "' but got nothing" << std::endl;
            break;
        }
        
        if (res.key != expected[i]) {
            std::cout << "ERROR: Expected '" << expected[i] << "', got '" << res.key << "'" << std::endl;
        } else {
            std::cout << "  Got: '" << res.key << "' (expected: '" << expected[i] << "') - PASS" << std::endl;
        }
    }
    
    // Should return no more results
    auto res = iter.previous();
    if (res.found) {
        std::cout << "ERROR: Expected no more results, but got '" << res.key << "'" << std::endl;
    } else {
        std::cout << "  No more results - PASS" << std::endl;
    }
}

void testReverseIteratorSeekPrefix() {
    std::cout << "\n=== Testing ReverseIterator SeekPrefix ===" << std::endl;
    
    Tree<std::string, std::string> tree;
    std::vector<std::string> keys = {"001", "002", "005", "010", "100"};
    
    for (const auto& key : keys) {
        auto [newTree, oldVal, didUpdate] = tree.insert(key, key);
        tree = newTree;
    }
    
    // Test cases: {prefix, expectResult}
    std::vector<std::pair<std::string, bool>> testCases = {
        {"005", true},   // existing prefix
        {"2", false}     // non-existing prefix
    };
    
    for (const auto& [prefix, expectResult] : testCases) {
        std::cout << "Testing prefix '" << prefix << "' (expect result: " 
                  << (expectResult ? "true" : "false") << ")" << std::endl;
        
        auto iter = ReverseIterator<std::string, std::string>(tree.getRoot());
        iter.seekPrefix(prefix);
        
        // Note: We need to check if the iterator is valid after seek
        // This is a simplified check - in a real implementation you'd need
        // to add a method to check if the iterator is valid
        bool hasResult = false;
        auto res = iter.previous();
        if (res.found) {
            hasResult = true;
        }
        
        if (expectResult && !hasResult) {
            std::cout << "  ERROR: Expected prefix '" << prefix << "' to exist" << std::endl;
        } else if (!expectResult && hasResult) {
            std::cout << "  ERROR: Unexpected result for prefix '" << prefix << "'" << std::endl;
        } else {
            std::cout << "  PASS" << std::endl;
        }
    }
}

void testReverseIteratorPrevious() {
    std::cout << "\n=== Testing ReverseIterator Previous ===" << std::endl;
    
    Tree<std::string, std::string> tree;
    std::vector<std::string> keys = {"001", "002", "005", "010", "100"};
    
    for (const auto& key : keys) {
        auto [newTree, oldVal, didUpdate] = tree.insert(key, key);
        tree = newTree;
    }
    
    auto iter = ReverseIterator<std::string, std::string>(tree.getRoot());
    
    // Iterate in reverse order
    for (int i = keys.size() - 1; i >= 0; i--) {
        auto res = iter.previous();
        if (!res.found) {
            std::cout << "ERROR: Expected key '" << keys[i] << "' but got nothing" << std::endl;
            break;
        }
        
        if (res.key != keys[i]) {
            std::cout << "ERROR: Expected '" << keys[i] << "', got '" << res.key << "'" << std::endl;
        } else {
            std::cout << "  Got: '" << res.key << "' (expected: '" << keys[i] << "') - PASS" << std::endl;
        }
    }
    
    // Should return no more results
    auto res = iter.previous();
    if (res.found) {
        std::cout << "ERROR: Expected no more results, but got '" << res.key << "'" << std::endl;
    } else {
        std::cout << "  No more results - PASS" << std::endl;
    }
}

int main() {
    testReverseIteratorBasic();
    testReverseIteratorSeekPrefix();
    testReverseIteratorPrevious();
    
    std::cout << "\n=== Reverse Iterator Tests Complete ===" << std::endl;
    return 0;
} 