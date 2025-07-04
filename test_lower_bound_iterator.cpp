#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include "radix/tree.hpp"

void testLowerBoundIterator() {
    std::cout << "Testing LowerBoundIterator..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Insert test data in sorted order
    std::vector<std::pair<std::string, std::string>> testData = {
        {"a", "alpha"},
        {"ab", "beta"},
        {"abc", "gamma"},
        {"abcd", "delta"},
        {"abcde", "epsilon"},
        {"b", "bravo"},
        {"bc", "charlie"},
        {"bcd", "delta"},
        {"c", "charlie"},
        {"d", "delta"},
        {"hello", "world"},
        {"hello/world", "test"},
        {"hello/world/test", "value"},
        {"hello/world/test/1", "one"},
        {"hello/world/test/2", "two"},
        {"hello/world/test/3", "three"},
        {"hello/other", "different"},
        {"hello/other/path", "another"},
        {"goodbye", "farewell"},
        {"goodbye/world", "test2"},
        {"x", "xray"},
        {"xy", "yankee"},
        {"xyz", "zulu"},
    };
    
    for (const auto& [key, value] : testData) {
        tree.insert(key, value);
    }
    
    std::cout << "Tree size: " << tree.len() << std::endl;
    
    // Helper function to collect all results from an iterator
    auto collectResults = [](LowerBoundIterator<std::string, std::string>& iter) {
        std::vector<std::pair<std::string, std::string>> results;
        while (true) {
            auto result = iter.next();
            if (!result.found) break;
            results.push_back({result.key, result.val});
        }
        return results;
    };
    
    // Test 1: Lower bound for exact match
    std::cout << "\nTest 1: Lower bound for exact match 'abc'" << std::endl;
    auto lbIter = tree.lowerBoundIterator("abc");
    lbIter.seekLowerBound("abc");
    auto results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'abc':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "abc");
    
    // Test 2: Lower bound for non-existent key between existing keys
    std::cout << "\nTest 2: Lower bound for 'abb' (between 'ab' and 'abc')" << std::endl;
    lbIter = tree.lowerBoundIterator("abb");
    lbIter.seekLowerBound("abb");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'abb':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "abc"); // Should find 'abc' as the lower bound
    
    // Test 3: Lower bound for key before all existing keys
    std::cout << "\nTest 3: Lower bound for '0' (before all keys)" << std::endl;
    lbIter = tree.lowerBoundIterator("0");
    lbIter.seekLowerBound("0");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from '0':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "a"); // Should find 'a' as the lower bound
    
    // Test 4: Lower bound for key after all existing keys
    std::cout << "\nTest 4: Lower bound for 'zzz' (after all keys)" << std::endl;
    lbIter = tree.lowerBoundIterator("zzz");
    lbIter.seekLowerBound("zzz");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'zzz':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(results.empty()); // Should find nothing
    
    // Test 5: Lower bound for empty string
    std::cout << "\nTest 5: Lower bound for empty string" << std::endl;
    lbIter = tree.lowerBoundIterator("");
    lbIter.seekLowerBound("");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from empty string:" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "a"); // Should find 'a' as the first key
    
    // Test 6: Lower bound for partial prefix
    std::cout << "\nTest 6: Lower bound for 'ab' (partial prefix)" << std::endl;
    lbIter = tree.lowerBoundIterator("ab");
    lbIter.seekLowerBound("ab");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'ab':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "ab"); // Should find 'ab' as the exact match
    
    // Test 7: Lower bound for 'hello' prefix
    std::cout << "\nTest 7: Lower bound for 'hello'" << std::endl;
    lbIter = tree.lowerBoundIterator("hello");
    lbIter.seekLowerBound("hello");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'hello':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "hello"); // Should find 'hello' as the exact match
    
    // Test 8: Lower bound for 'hello/world/test/1.5' (between test/1 and test/2)
    std::cout << "\nTest 8: Lower bound for 'hello/world/test/1.5'" << std::endl;
    lbIter = tree.lowerBoundIterator("hello/world/test/1.5");
    lbIter.seekLowerBound("hello/world/test/1.5");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'hello/world/test/1.5':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "hello/world/test/2"); // Should find test/2 as the lower bound
    
    // Test 9: Lower bound for 'hello/world/test/4' (after all test items)
    std::cout << "\nTest 9: Lower bound for 'hello/world/test/4'" << std::endl;
    lbIter = tree.lowerBoundIterator("hello/world/test/4");
    lbIter.seekLowerBound("hello/world/test/4");
    results = collectResults(lbIter);
    std::cout << "Found " << results.size() << " items starting from 'hello/world/test/4':" << std::endl;
    for (const auto& [key, val] : results) {
        std::cout << "  " << key << " -> " << val << std::endl;
    }
    assert(!results.empty());
    assert(results[0].first == "hello/other"); // Should find the next branch
    
    // Test 10: Verify all results are in sorted order
    std::cout << "\nTest 10: Verify all results are in sorted order" << std::endl;
    lbIter = tree.lowerBoundIterator("a");
    lbIter.seekLowerBound("a");
    results = collectResults(lbIter);
    
    std::vector<std::string> keys;
    for (const auto& [key, val] : results) {
        keys.push_back(key);
    }
    
    std::vector<std::string> sortedKeys = keys;
    std::sort(sortedKeys.begin(), sortedKeys.end());
    
    bool isSorted = (keys == sortedKeys);
    std::cout << "Results are " << (isSorted ? "sorted" : "not sorted") << std::endl;
    assert(isSorted);
    
    std::cout << "\nAll LowerBoundIterator tests passed!" << std::endl;
}

int main() {
    testLowerBoundIterator();
    return 0;
} 