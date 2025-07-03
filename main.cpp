//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "radix/tree.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>
#include <random>
#include <algorithm>

int main() {
    // Example 1: Using std::vector<uint8_t> as key type and std::string as value type
    std::cout << "Example 1: Using std::vector<uint8_t> as key type and std::string as value type" << std::endl;
    
    // Create a radix tree with vector<uint8_t> keys and string values
    Tree<std::vector<uint8_t>, std::string> tree1;
    
    // Insert some key-value pairs
    std::vector<uint8_t> key1 = {'h', 'e', 'l', 'l', 'o'};
    std::vector<uint8_t> key2 = {'h', 'e', 'l', 'p'};
    std::vector<uint8_t> key3 = {'w', 'o', 'r', 'l', 'd'};
    
    auto [tree1_1, oldVal1, didUpdate1] = tree1.insert(key1, "greeting");
    tree1 = tree1_1;
    auto [tree1_2, oldVal2, didUpdate2] = tree1.insert(key2, "assistance");
    tree1 = tree1_2;
    auto [tree1_3, oldVal3, didUpdate3] = tree1.insert(key3, "planet");
    tree1 = tree1_3;
    
    // Look up values
    auto val1 = tree1.Get(key1);
    auto val2 = tree1.Get(key2);
    auto val3 = tree1.Get(key3);
    
    std::cout << "Value for 'hello': " << (val1 ? *val1 : "not found") << std::endl;
    std::cout << "Value for 'help': " << (val2 ? *val2 : "not found") << std::endl;
    std::cout << "Value for 'world': " << (val3 ? *val3 : "not found") << std::endl;
    
    // Delete a key
    auto [tree1_4, oldVal4, found4] = tree1.del(key2);
    tree1 = tree1_4;
    auto val2_after_delete = tree1.Get(key2);
    std::cout << "Value for 'help' after delete: " << (val2_after_delete ? *val2_after_delete : "not found") << std::endl;
    
    // Example 2: Using std::string as key type and int as value type
    std::cout << "\nExample 2: Using std::string as key type and int as value type" << std::endl;
    
    // Create a radix tree with string keys and int values
    Tree<std::string, int> tree2;
    
    // Insert some key-value pairs
    auto [tree2_1, oldVal5, didUpdate5] = tree2.insert("apple", 1);
    tree2 = tree2_1;
    auto [tree2_2, oldVal6, didUpdate6] = tree2.insert("banana", 2);
    tree2 = tree2_2;
    auto [tree2_3, oldVal7, didUpdate7] = tree2.insert("cherry", 3);
    tree2 = tree2_3;
    
    // Look up values
    auto val4 = tree2.Get("apple");
    auto val5 = tree2.Get("banana");
    auto val6 = tree2.Get("cherry");
    
    std::cout << "Value for 'apple': " << (val4 ? std::to_string(*val4) : "not found") << std::endl;
    std::cout << "Value for 'banana': " << (val5 ? std::to_string(*val5) : "not found") << std::endl;
    std::cout << "Value for 'cherry': " << (val6 ? std::to_string(*val6) : "not found") << std::endl;
    
    // Example 3: Using the iterator
    std::cout << "\nExample 3: Using the iterator" << std::endl;
    
    // Create an iterator
    auto it = tree2.iterator();
    
    // Iterate over all key-value pairs
    std::cout << "All key-value pairs:" << std::endl;
    while (true) {
        auto result = it.next();
        if (!result.found) break;
        
        std::cout << "  " << result.key << ": " << result.val << std::endl;
    }
    
    // Example 4: Using prefix search
    std::cout << "\nExample 4: Using prefix search" << std::endl;
    
    // Insert more keys with common prefix
    auto [tree2_4, oldVal8, didUpdate8] = tree2.insert("apricot", 4);
    tree2 = tree2_4;
    auto [tree2_5, oldVal9, didUpdate9] = tree2.insert("apartment", 5);
    tree2 = tree2_5;
    
    // Create an iterator and seek to a prefix
    auto it2 = tree2.iterator();
    it2.seekPrefix("ap");
    
    // Iterate over all key-value pairs with the prefix
    std::cout << "All key-value pairs with prefix 'ap':" << std::endl;
    while (true) {
        auto result = it2.next();
        if (!result.found) break;
        
        std::cout << "  " << result.key << ": " << result.val << std::endl;
    }

    // Example 5: Using reverse iterator
    std::cout << "\nExample 5: Using reverse iterator" << std::endl;
    
    // Create a reverse iterator
    auto rit = ReverseIterator<std::string, int>(tree2.getRoot());
    
    // Seek to a lower bound
    
    // Iterate backwards from the lower bound
    std::cout << "All key-value pairs in reverse order':" << std::endl;
    while (true) {
        auto result = rit.previous();
        if (!result.found) break;
        std::cout << "  " << result.key << ": " << result.val << std::endl;
    }

    // Example 6: Testing with words from words.txt
    std::cout << "\nExample 6: Testing Tree with words from words.txt" << std::endl;
    
    // Create a radix tree with string keys and int values
    Tree<std::string, int> wordTree;
    std::vector<std::string> words;
    
    // Read words from file
    std::ifstream file("words.txt");
    if (!file.is_open()) {
        std::cerr << "Error: Could not open words.txt" << std::endl;
        return 1;
    }
    
    std::string word;
    int value = 1;
    while (std::getline(file, word)) {
        // Remove any trailing whitespace
        word.erase(word.find_last_not_of(" \n\r\t") + 1);
        if (!word.empty()) {
            words.push_back(word);
            auto [newTree, oldVal, didUpdate] = wordTree.insert(word, value);
            wordTree = newTree;
            value++;
        }
    }
    file.close();
    
    std::cout << "\nVerifying all inserted words:" << std::endl;
    int totalWords = 0;
    int foundWords = 0;

    int counter = 1;
    for (const auto& word : words) {
        totalWords++;
        auto val = wordTree.Get(word);
        if (val) {
            foundWords++;
            if (val != counter) {
                std::cout << "NOTOK";
                std::cout << "ERROR: Value mismatch for word '" << word << "'!" << *val << " " << counter << std::endl;
            }
            counter++;
            std::cout << word << ": " << *val << std::endl;
        } else {
            std::cout << "ERROR: Word '" << word << "' not found!" << std::endl;
        }
    }
    
    std::cout << "\nSummary:" << std::endl;
    std::cout << "Total words processed: " << totalWords << std::endl;
    std::cout << "Words found in tree: " << foundWords << std::endl;
    std::cout << "Success rate: " << (foundWords * 100.0 / totalWords) << "%" << std::endl;
    
    // Test GetLeavesInSubtree function
    std::cout << "\n=== Testing GetLeavesInSubtree ===" << std::endl;
    Tree<std::string, std::string> test_tree;
    std::cout << "Empty tree leaves: " << test_tree.GetLeavesInSubtree() << std::endl;
    
    auto [test_tree1, old1, updated1] = test_tree.insert("hello", "world");
    std::cout << "After inserting 'hello': " << test_tree1.GetLeavesInSubtree() << std::endl;
    
    auto [test_tree2, old2, updated2] = test_tree1.insert("world", "hello");
    std::cout << "After inserting 'world': " << test_tree2.GetLeavesInSubtree() << std::endl;
    
    auto [test_tree3, old3, updated3] = test_tree2.insert("hello", "updated");
    std::cout << "After updating 'hello': " << test_tree3.GetLeavesInSubtree() << std::endl;
    
    std::cout << "Tree size: " << test_tree3.len() << std::endl;
    
    // Example 7: Testing LongestPrefix function
    std::cout << "\n=== Testing LongestPrefix Function ===" << std::endl;
    
    // Create a test tree for longest prefix testing
    Tree<std::string, std::string> longestPrefixTree;
    
    // Insert keys with common prefixes (including empty string for fallback)
    auto [lpt1, lptold1, lptupd1] = longestPrefixTree.insert("", "empty");
    longestPrefixTree = lpt1;
    auto [lpt2, lptold2, lptupd2] = longestPrefixTree.insert("hello", "greeting");
    longestPrefixTree = lpt2;
    auto [lpt3, lptold3, lptupd3] = longestPrefixTree.insert("help", "assistance");
    longestPrefixTree = lpt3;
    auto [lpt4, lptold4, lptupd4] = longestPrefixTree.insert("world", "planet");
    longestPrefixTree = lpt4;
    auto [lpt5, lptold5, lptupd5] = longestPrefixTree.insert("work", "job");
    longestPrefixTree = lpt5;
    auto [lpt6, lptold6, lptupd6] = longestPrefixTree.insert("worker", "employee");
    longestPrefixTree = lpt6;
    auto [lpt7, lptold7, lptupd7] = longestPrefixTree.insert("working", "active");
    longestPrefixTree = lpt7;
    
    std::cout << "Inserted keys: (empty), hello, help, world, work, worker, working" << std::endl;
    
    // Test cases for longest prefix
    std::vector<std::pair<std::string, std::string>> testCases = {
        {"hello", "hello"},           // Exact match
        {"help", "help"},             // Exact match
        {"hel", "help"},              // Should find "help" (longest prefix)
        {"he", "help"},               // Should find "help" (longest prefix)
        {"h", "help"},                // Should find "help" (longest prefix)
        {"world", "world"},           // Exact match
        {"wor", "work"},              // Should find "work" (longest prefix)
        {"work", "work"},             // Exact match
        {"worker", "worker"},         // Exact match
        {"working", "working"},       // Exact match
        {"worki", "working"},         // Should find "working" (longest prefix)
        {"worke", "worker"},          // Should find "worker" (longest prefix)
        {"xyz", ""},                  // No match, should return empty string
        {"", ""},                     // Empty string
        {"a", ""},                    // No match, should return empty string
        {"z", ""},                    // No match, should return empty string
    };
    
    for (const auto& [searchKey, expectedKey] : testCases) {
        auto result = longestPrefixTree.LongestPrefix(searchKey);
        std::cout << "Search: '" << searchKey << "' -> ";
        
        if (result.found) {
            std::cout << "Found: '" << result.key << "' = '" << result.val << "'";
            if (expectedKey.empty()) {
                std::cout << " (UNEXPECTED - should not have found anything)";
            } else if (result.key != expectedKey) {
                std::cout << " (UNEXPECTED - expected '" << expectedKey << "')";
            } else {
                std::cout << " (CORRECT)";
            }
        } else {
            std::cout << "Not found";
            if (!expectedKey.empty()) {
                std::cout << " (UNEXPECTED - should have found '" << expectedKey << "')";
            } else {
                std::cout << " (CORRECT)";
            }
        }
        std::cout << std::endl;
    }
    
    // Test with vector<uint8_t> keys
    std::cout << "\n=== Testing LongestPrefix with vector<uint8_t> keys ===" << std::endl;
    
    Tree<std::vector<uint8_t>, std::string> vectorTree;
    
    // Insert some test data
    std::vector<uint8_t> v1 = {'h', 'e', 'l', 'l', 'o'};
    std::vector<uint8_t> v2 = {'h', 'e', 'l', 'p'};
    std::vector<uint8_t> v3 = {'w', 'o', 'r', 'k'};
    std::vector<uint8_t> v4 = {'w', 'o', 'r', 'k', 'e', 'r'};
    
    auto [vt1, vtold1, vtupd1] = vectorTree.insert(v1, "greeting");
    vectorTree = vt1;
    auto [vt2, vtold2, vtupd2] = vectorTree.insert(v2, "assistance");
    vectorTree = vt2;
    auto [vt3, vtold3, vtupd3] = vectorTree.insert(v3, "job");
    vectorTree = vt3;
    auto [vt4, vtold4, vtupd4] = vectorTree.insert(v4, "employee");
    vectorTree = vt4;
    
    // Test vector longest prefix
    std::vector<uint8_t> searchVec = {'h', 'e', 'l'};
    auto vectorResult = vectorTree.LongestPrefix(searchVec);
    
    std::cout << "Search vector: [h, e, l] -> ";
    if (vectorResult.found) {
        std::cout << "Found: [";
        for (size_t i = 0; i < vectorResult.key.size(); ++i) {
            if (i > 0) std::cout << ", ";
            std::cout << "'" << (char)vectorResult.key[i] << "'";
        }
        std::cout << "] = '" << vectorResult.val << "'";
    } else {
        std::cout << "Not found";
    }
    std::cout << std::endl;
    
    // Test edge cases
    std::cout << "\n=== Testing LongestPrefix Edge Cases ===" << std::endl;
    
    // Test with empty tree
    Tree<std::string, std::string> emptyTree;
    auto emptyResult = emptyTree.LongestPrefix("anything");
    std::cout << "Empty tree search 'anything': " << (emptyResult.found ? "FOUND" : "Not found") << std::endl;
    
    // Test with single character keys
    Tree<std::string, std::string> singleCharTree;
    auto [sct1, sctold1, sctupd1] = singleCharTree.insert("a", "first");
    singleCharTree = sct1;
    auto [sct2, sctold2, sctupd2] = singleCharTree.insert("b", "second");
    singleCharTree = sct2;
    
    auto singleResult = singleCharTree.LongestPrefix("a");
    std::cout << "Single char tree search 'a': " << (singleResult.found ? "FOUND '" + singleResult.key + "'" : "Not found") << std::endl;
    
    auto noMatchResult = singleCharTree.LongestPrefix("c");
    std::cout << "Single char tree search 'c': " << (noMatchResult.found ? "FOUND '" + noMatchResult.key + "'" : "Not found") << std::endl;
    
    // Test case matching the Go test
    std::cout << "\n=== Testing LongestPrefix with Go test cases ===" << std::endl;
    
    Tree<std::string, std::string> goTestTree;
    
    // Insert the same keys as in the Go test
    std::vector<std::string> goKeys = {"", "foo", "foobar", "foobarbaz", "foobarbazzip", "foozip"};
    for (const auto& key : goKeys) {
        auto [newTree, _, __] = goTestTree.insert(key, "value");
        goTestTree = newTree;
    }
    
    std::cout << "Inserted keys: ";
    for (const auto& key : goKeys) {
        std::cout << "'" << key << "' ";
    }
    std::cout << std::endl;
    
    // Test cases from the Go test
    std::vector<std::pair<std::string, std::string>> goTestCases = {
        {"a", ""},
        {"abc", ""},
        {"fo", ""},
        {"foo", "foo"},
        {"foob", "foo"},
        {"foobar", "foobar"},
        {"foobarba", "foobar"},
        {"foobarbaz", "foobarbaz"},
        {"foobarbazzi", "foobarbaz"},
        {"foobarbazzip", "foobarbazzip"},
        {"foozi", "foo"},
        {"foozip", "foozip"},
        {"foozipzap", "foozip"},
    };
    
    for (const auto& [input, expected] : goTestCases) {
        auto result = goTestTree.LongestPrefix(input);
        std::cout << "Search: '" << input << "' -> ";
        
        if (result.found) {
            std::cout << "Found: '" << result.key << "'";
            if (result.key != expected) {
                std::cout << " (UNEXPECTED - expected '" << expected << "')";
            } else {
                std::cout << " (CORRECT)";
            }
        } else {
            std::cout << "Not found";
            if (!expected.empty()) {
                std::cout << " (UNEXPECTED - should have found '" << expected << "')";
            } else {
                std::cout << " (CORRECT)";
            }
        }
        std::cout << std::endl;
    }
    
    Tree<std::string, std::string> tree;
    
    // Insert some test data
    tree.insert("apple", "fruit1");
    tree.insert("banana", "fruit2");
    tree.insert("cherry", "fruit3");
    tree.insert("date", "fruit4");
    tree.insert("elderberry", "fruit5");
    
    std::cout << "Tree size: " << tree.len() << std::endl;
    std::cout << "Leaves in subtree: " << tree.GetLeavesInSubtree() << std::endl;
    
    // Test GetAtIndex functionality
    for (int i = 0; i < tree.GetLeavesInSubtree(); i++) {
        auto [key, value, found] = tree.GetAtIndex(i);
        if (found) {
            std::cout << "Index " << i << ": key='" << key << "', value='" << value << "'" << std::endl;
        } else {
            std::cout << "Index " << i << ": not found" << std::endl;
        }
    }
    
    // Test out of bounds
    auto [key_out, value_out, found_out] = tree.GetAtIndex(10);
    std::cout << "Index 10 (out of bounds): found=" << found_out << std::endl;
    
    // Test GetAtIndex with shuffled insertion
    std::cout << "\n=== Testing GetAtIndex with shuffled insertion ===" << std::endl;
    
    // Read all words from words.txt
    std::ifstream wordsFile("words.txt");
    if (!wordsFile.is_open()) {
        std::cerr << "Error: Could not open words.txt" << std::endl;
        return 1;
    }
    
    std::vector<std::string> allWords;
    std::string testWord;
    while (std::getline(wordsFile, testWord)) {
        // Remove any trailing whitespace
        testWord.erase(testWord.find_last_not_of(" \n\r\t") + 1);
        if (!testWord.empty()) {
            allWords.push_back(testWord);
        }
    }
    wordsFile.close();
    
    std::cout << "Total words read: " << allWords.size() << std::endl;
    
    // Create a copy for shuffling
    std::vector<std::string> shuffledWords = allWords;
    
    // Shuffle the words
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(shuffledWords.begin(), shuffledWords.end(), g);
    
    std::cout << "First 10 shuffled words: ";
    for (int i = 0; i < std::min(10, (int)shuffledWords.size()); i++) {
        std::cout << shuffledWords[i] << " ";
    }
    std::cout << std::endl;
    
    // Insert shuffled words into tree
    Tree<std::string, std::string> shuffledTree;
    for (const auto& w : shuffledWords) {
        auto [newTree, oldVal, didUpdate] = shuffledTree.insert(w, w);
        shuffledTree = newTree;
    }
    
    std::cout << "Tree size after insertion: " << shuffledTree.len() << std::endl;
    std::cout << "Leaves in subtree: " << shuffledTree.GetLeavesInSubtree() << std::endl;
    
    // Sort the original words for comparison
    std::sort(allWords.begin(), allWords.end());
    
    // Verify GetAtIndex returns words in sorted order
    bool allCorrect = true;
    int numCorrect = 0;
    
    for (int idx = 0; idx < (int)allWords.size(); idx++) {
        auto [key, value, found] = shuffledTree.GetAtIndex(idx);
        if (found) {
            if (key != allWords[idx]) {
                std::cout << "MISMATCH at index " << idx << ": expected '" << allWords[idx] << "', got '" << key << "'" << std::endl;
                allCorrect = false;
            } else {
                numCorrect++;
            }
        } else {
            std::cout << "NOT FOUND at index " << idx << std::endl;
            allCorrect = false;
        }
    }
    
    std::cout << "Correct matches: " << numCorrect << "/" << allWords.size() << std::endl;
    std::cout << "All indices correct: " << (allCorrect ? "YES" : "NO") << std::endl;
    
    // Show first 10 words from GetAtIndex
    std::cout << "First 10 words from GetAtIndex: ";
    for (int i = 0; i < std::min(10, (int)allWords.size()); i++) {
        auto [key, value, found] = shuffledTree.GetAtIndex(i);
        if (found) {
            std::cout << key << " ";
        }
    }
    std::cout << std::endl;
    
    return 0;
}
