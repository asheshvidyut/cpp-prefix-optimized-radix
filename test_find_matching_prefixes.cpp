#include <iostream>
#include <string>
#include <vector>
#include <cassert>
#include <algorithm>
#include "radix/tree.hpp"

void testFindMatchingPrefixes() {
    std::cout << "Testing findMatchingPrefixes..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Insert test data with various prefix relationships
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
    
    // Helper function to print results
    auto printResults = [](const std::vector<std::pair<std::string, std::string>>& results, const std::string& searchKey) {
        std::cout << "Prefixes of '" << searchKey << "':" << std::endl;
        if (results.empty()) {
            std::cout << "  No matching prefixes found" << std::endl;
        } else {
            for (const auto& [key, val] : results) {
                std::cout << "  " << key << " -> " << val << std::endl;
            }
        }
        std::cout << "Total: " << results.size() << " matches" << std::endl;
    };
    
    // Test 1: Find prefixes of "abcde"
    std::cout << "\nTest 1: Find prefixes of 'abcde'" << std::endl;
    auto results = tree.findMatchingPrefixes("abcde");
    printResults(results, "abcde");
    assert(results.size() == 5);
    assert(results[0].first == "a");
    assert(results[1].first == "ab");
    assert(results[2].first == "abc");
    assert(results[3].first == "abcd");
    assert(results[4].first == "abcde");
    
    // Test 2: Find prefixes of "abc"
    std::cout << "\nTest 2: Find prefixes of 'abc'" << std::endl;
    results = tree.findMatchingPrefixes("abc");
    printResults(results, "abc");
    assert(results.size() == 3);
    assert(results[0].first == "a");
    assert(results[1].first == "ab");
    assert(results[2].first == "abc");
    
    // Test 3: Find prefixes of "hello/world/test/2"
    std::cout << "\nTest 3: Find prefixes of 'hello/world/test/2'" << std::endl;
    results = tree.findMatchingPrefixes("hello/world/test/2");
    printResults(results, "hello/world/test/2");
    assert(results.size() == 4);
    assert(results[0].first == "hello");
    assert(results[1].first == "hello/world");
    assert(results[2].first == "hello/world/test");
    assert(results[3].first == "hello/world/test/2");
    
    // Test 4: Find prefixes of "hello/world/test/1.5" (non-existent key)
    std::cout << "\nTest 4: Find prefixes of 'hello/world/test/1.5' (non-existent)" << std::endl;
    results = tree.findMatchingPrefixes("hello/world/test/1.5");
    printResults(results, "hello/world/test/1.5");
    assert(results.size() == 3);
    assert(results[0].first == "hello");
    assert(results[1].first == "hello/world");
    assert(results[2].first == "hello/world/test");
    
    // Test 5: Find prefixes of "hello/other/path"
    std::cout << "\nTest 5: Find prefixes of 'hello/other/path'" << std::endl;
    results = tree.findMatchingPrefixes("hello/other/path");
    printResults(results, "hello/other/path");
    assert(results.size() == 3);
    assert(results[0].first == "hello");
    assert(results[1].first == "hello/other");
    assert(results[2].first == "hello/other/path");
    
    // Test 6: Find prefixes of "xyz"
    std::cout << "\nTest 6: Find prefixes of 'xyz'" << std::endl;
    results = tree.findMatchingPrefixes("xyz");
    printResults(results, "xyz");
    assert(results.size() == 3);
    assert(results[0].first == "x");
    assert(results[1].first == "xy");
    assert(results[2].first == "xyz");
    
    // Test 7: Find prefixes of "a" (single character)
    std::cout << "\nTest 7: Find prefixes of 'a'" << std::endl;
    results = tree.findMatchingPrefixes("a");
    printResults(results, "a");
    assert(results.size() == 1);
    assert(results[0].first == "a");
    
    // Test 8: Find prefixes of "nonexistent" (no matches)
    std::cout << "\nTest 8: Find prefixes of 'nonexistent' (no matches)" << std::endl;
    results = tree.findMatchingPrefixes("nonexistent");
    printResults(results, "nonexistent");
    assert(results.empty());
    
    // Test 9: Find prefixes of empty string
    std::cout << "\nTest 9: Find prefixes of empty string" << std::endl;
    results = tree.findMatchingPrefixes("");
    printResults(results, "");
    assert(results.empty());
    
    // Test 10: Find prefixes of "hello/world/test/4" (after all test items)
    std::cout << "\nTest 10: Find prefixes of 'hello/world/test/4'" << std::endl;
    results = tree.findMatchingPrefixes("hello/world/test/4");
    printResults(results, "hello/world/test/4");
    assert(results.size() == 3);
    assert(results[0].first == "hello");
    assert(results[1].first == "hello/world");
    assert(results[2].first == "hello/world/test");
    
    // Test 11: Find prefixes of "hello/world/test/1.5" (between test/1 and test/2)
    std::cout << "\nTest 11: Find prefixes of 'hello/world/test/1.5'" << std::endl;
    results = tree.findMatchingPrefixes("hello/world/test/1.5");
    printResults(results, "hello/world/test/1.5");
    assert(results.size() == 3);
    assert(results[0].first == "hello");
    assert(results[1].first == "hello/world");
    assert(results[2].first == "hello/world/test");
    
    // Test 12: Verify results are in ascending order
    std::cout << "\nTest 12: Verify results are in ascending order" << std::endl;
    results = tree.findMatchingPrefixes("abcde");
    std::vector<std::string> keys;
    for (const auto& [key, val] : results) {
        keys.push_back(key);
    }
    
    std::vector<std::string> sortedKeys = keys;
    std::sort(sortedKeys.begin(), sortedKeys.end());
    
    bool isSorted = (keys == sortedKeys);
    std::cout << "Results are " << (isSorted ? "sorted" : "not sorted") << std::endl;
    assert(isSorted);
    
    // Test 13: Search for a key that exists
    auto result1 = tree.findMatchingPrefixes("cherry");
    assert(result1.size() == 1);
    assert(result1[0].first == "cherry");
    assert(result1[0].second == "fruit3");
    std::cout << "✓ Test 13 passed: Exact match" << std::endl;
    
    // Test 14: Search for a key that doesn't exist but has a lower bound
    auto result2 = tree.findMatchingPrefixes("blueberry");
    assert(result2.size() == 0);  // No keys are prefixes of "blueberry"
    std::cout << "✓ Test 14 passed: No matching prefixes" << std::endl;
    
    // Test 15: Search for a key that doesn't exist and is greater than all keys
    auto result3 = tree.findMatchingPrefixes("zucchini");
    assert(result3.size() == 0);  // No keys are prefixes of "zucchini"
    std::cout << "✓ Test 15 passed: Greater than all keys" << std::endl;
    
    // Test 16: Search for a key that doesn't exist but is between existing keys
    auto result4 = tree.findMatchingPrefixes("coconut");
    assert(result4.size() == 0);  // No keys are prefixes of "coconut"
    std::cout << "✓ Test 16 passed: Between existing keys" << std::endl;
    
    // Test 17: Search for a key that doesn't exist and is less than all keys
    auto result5 = tree.findMatchingPrefixes("aardvark");
    assert(result5.size() == 0);  // No keys are prefixes of "aardvark"
    std::cout << "✓ Test 17 passed: Less than all keys" << std::endl;
    
    // Test 18: Search for empty string
    auto result6 = tree.findMatchingPrefixes("");
    assert(result6.size() == 0);  // Empty string should return no results
    std::cout << "✓ Test 18 passed: Empty string search" << std::endl;
    
    std::cout << "\nAll findMatchingPrefixes tests passed!" << std::endl;
}

int main() {
    testFindMatchingPrefixes();
    return 0;
} 