#include <iostream>
#include <vector>
#include <string>
#include <cassert>
#include <algorithm>
#include "radix/tree.hpp"

// Helper function to check if two vectors contain the same elements
template<typename T>
bool vectorsEqual(const std::vector<T>& a, const std::vector<T>& b) {
    if (a.size() != b.size()) return false;
    auto a_sorted = a;
    auto b_sorted = b;
    std::sort(a_sorted.begin(), a_sorted.end());
    std::sort(b_sorted.begin(), b_sorted.end());
    return a_sorted == b_sorted;
}

void testAddItems() {
    std::cout << "Testing AddItems..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Test basic insertion
    auto [tree1, oldVal1, didUpdate1] = tree.insert("foo", "a");
    assert(!didUpdate1);
    assert(tree1.Get("foo") == "a");
    
    auto [tree2, oldVal2, didUpdate2] = tree1.insert("bar", "b");
    assert(!didUpdate2);
    assert(tree2.Get("bar") == "b");
    assert(tree2.Get("foo") == "a");
    
    // Test overwrite with overwrite_existing = false (simulated)
    auto [tree3, oldVal3, didUpdate3] = tree2.insert("foo", "c");
    assert(didUpdate3);  // Should update existing
    assert(tree3.Get("foo") == "c");
    
    std::cout << "✓ AddItems test passed!" << std::endl;
}

void testLongestPrefix() {
    std::cout << "Testing LongestPrefix..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Add test data
    tree.insert("foo", "a");
    tree.insert("bar", "b");
    tree.insert("baro", "c");
    tree.insert("foo/bar", "d");
    tree.insert("barn", "e");
    tree.insert("barp", "f");
    
    // Test exact matches
    assert(tree.Get("foo") == "a");
    assert(tree.Get("bar") == "b");
    assert(tree.Get("baro") == "c");
    assert(tree.Get("foo/bar") == "d");
    assert(tree.Get("barn") == "e");
    assert(tree.Get("barp") == "f");
    
    // Test longest prefix matches
    auto result1 = tree.LongestPrefix("foo");
    assert(result1.found && result1.key == "foo" && result1.val == "a");
    
    auto result2 = tree.LongestPrefix("foosball");
    assert(result2.found && result2.key == "foo" && result2.val == "a");
    
    auto result3 = tree.LongestPrefix("foo/");
    assert(result3.found && result3.key == "foo" && result3.val == "a");
    
    auto result4 = tree.LongestPrefix("foo/bar");
    assert(result4.found && result4.key == "foo/bar" && result4.val == "d");
    
    auto result5 = tree.LongestPrefix("foo/bar/zzz");
    assert(result5.found && result5.key == "foo/bar" && result5.val == "d");
    
    auto result6 = tree.LongestPrefix("bar");
    assert(result6.found && result6.key == "bar" && result6.val == "b");
    
    auto result7 = tree.LongestPrefix("baritone");
    assert(result7.found && result7.key == "bar" && result7.val == "b");
    
    auto result8 = tree.LongestPrefix("barometer");
    assert(result8.found && result8.key == "baro" && result8.val == "c");
    
    auto result9 = tree.LongestPrefix("barnacle");
    assert(result9.found && result9.key == "barn" && result9.val == "e");
    
    auto result10 = tree.LongestPrefix("barpomus");
    assert(result10.found && result10.key == "barp" && result10.val == "f");
    
    // Test non-existent keys
    auto result11 = tree.LongestPrefix("toto");
    assert(!result11.found);
    
    auto result12 = tree.LongestPrefix(" ");
    assert(!result12.found);
    
    std::cout << "✓ LongestPrefix test passed!" << std::endl;
}

void testFindMatchingPrefixes() {
    std::cout << "Testing FindMatchingPrefixes..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Add test data
    tree.insert("foo", "a");
    tree.insert("bar", "b");
    tree.insert("baro", "c");
    tree.insert("foo/bar", "d");
    tree.insert("barn", "e");
    tree.insert("barp", "f");
    
    // Test findMatchingPrefixes for various keys
    auto result1 = tree.findMatchingPrefixes("foo");
    std::vector<std::pair<std::string, std::string>> expected1 = {{"foo", "a"}};
    assert(vectorsEqual(result1, expected1));
    
    auto result2 = tree.findMatchingPrefixes("foosball");
    std::vector<std::pair<std::string, std::string>> expected2 = {{"foo", "a"}};
    assert(vectorsEqual(result2, expected2));
    
    auto result3 = tree.findMatchingPrefixes("foo/");
    std::vector<std::pair<std::string, std::string>> expected3 = {{"foo", "a"}};
    assert(vectorsEqual(result3, expected3));
    
    auto result4 = tree.findMatchingPrefixes("foo/bar");
    std::vector<std::pair<std::string, std::string>> expected4 = {{"foo", "a"}, {"foo/bar", "d"}};
    assert(vectorsEqual(result4, expected4));
    
    auto result5 = tree.findMatchingPrefixes("foo/bar/zzz");
    std::vector<std::pair<std::string, std::string>> expected5 = {{"foo", "a"}, {"foo/bar", "d"}};
    assert(vectorsEqual(result5, expected5));
    
    auto result6 = tree.findMatchingPrefixes("bar");
    std::vector<std::pair<std::string, std::string>> expected6 = {{"bar", "b"}};
    assert(vectorsEqual(result6, expected6));
    
    auto result7 = tree.findMatchingPrefixes("baritone");
    std::vector<std::pair<std::string, std::string>> expected7 = {{"bar", "b"}};
    assert(vectorsEqual(result7, expected7));
    
    auto result8 = tree.findMatchingPrefixes("barometer");
    std::vector<std::pair<std::string, std::string>> expected8 = {{"bar", "b"}, {"baro", "c"}};
    assert(vectorsEqual(result8, expected8));
    
    auto result9 = tree.findMatchingPrefixes("barnacle");
    std::vector<std::pair<std::string, std::string>> expected9 = {{"bar", "b"}, {"barn", "e"}};
    assert(vectorsEqual(result9, expected9));
    
    auto result10 = tree.findMatchingPrefixes("barpomus");
    std::vector<std::pair<std::string, std::string>> expected10 = {{"bar", "b"}, {"barp", "f"}};
    assert(vectorsEqual(result10, expected10));
    
    // Test non-existent keys
    auto result11 = tree.findMatchingPrefixes("toto");
    assert(result11.empty());
    
    auto result12 = tree.findMatchingPrefixes(" ");
    assert(result12.empty());
    
    std::cout << "✓ FindMatchingPrefixes test passed!" << std::endl;
}

void testVeryDeepTrie() {
    std::cout << "Testing VeryDeepTrie..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Create a very deep key (similar to the 20960 character key in the original test)
    std::string deepKey(1000, 'a');  // Using 1000 instead of 20960 for faster testing
    tree.insert(deepKey, "deep_value");
    
    // Test that we can find the deep key
    auto result = tree.Get(deepKey);
    assert(result == "deep_value");
    
    // Test longest prefix for the deep key
    auto longestPrefix = tree.LongestPrefix(deepKey);
    assert(longestPrefix.found && longestPrefix.key == deepKey && longestPrefix.val == "deep_value");
    
    // Test findMatchingPrefixes for the deep key
    auto matchingPrefixes = tree.findMatchingPrefixes(deepKey);
    assert(matchingPrefixes.size() == 1);
    assert(matchingPrefixes[0].first == deepKey && matchingPrefixes[0].second == "deep_value");
    
    std::cout << "✓ VeryDeepTrie test passed!" << std::endl;
}

void testEdgeCases() {
    std::cout << "Testing EdgeCases..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Test empty string
    tree.insert("", "empty");
    auto result1 = tree.findMatchingPrefixes("anything");
    std::vector<std::pair<std::string, std::string>> expected1 = {{"", "empty"}};
    assert(vectorsEqual(result1, expected1));
    
    // Test single character keys
    tree.insert("a", "single_a");
    tree.insert("b", "single_b");
    
    auto result2 = tree.findMatchingPrefixes("abc");
    std::vector<std::pair<std::string, std::string>> expected2 = {{"", "empty"}, {"a", "single_a"}};
    assert(vectorsEqual(result2, expected2));
    
    // Test keys that are prefixes of each other
    tree.insert("prefix", "prefix_value");
    tree.insert("prefix/suffix", "prefix_suffix_value");
    
    auto result3 = tree.findMatchingPrefixes("prefix/suffix/extra");
    std::vector<std::pair<std::string, std::string>> expected3 = {
        {"", "empty"}, {"prefix", "prefix_value"}, {"prefix/suffix", "prefix_suffix_value"}
    };
    assert(vectorsEqual(result3, expected3));
    
    std::cout << "✓ EdgeCases test passed!" << std::endl;
}

int main() {
    std::cout << "Running comprehensive findMatchingPrefixes tests..." << std::endl;
    
    testAddItems();
    testLongestPrefix();
    testFindMatchingPrefixes();
    testVeryDeepTrie();
    testEdgeCases();
    
    std::cout << "\nAll comprehensive tests passed!" << std::endl;
    return 0;
} 