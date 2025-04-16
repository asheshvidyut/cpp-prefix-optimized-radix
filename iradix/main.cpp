//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "radix.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <sstream>

int main() {
    // Example 1: Using std::vector<uint8_t> as key type and std::string as value type
    std::cout << "Example 1: Using std::vector<uint8_t> as key type and std::string as value type" << std::endl;
    
    // Create a radix tree with vector<uint8_t> keys and string values
    RadixTree<std::vector<uint8_t>, std::string> tree1;
    
    // Insert some key-value pairs
    std::vector<uint8_t> key1 = {'h', 'e', 'l', 'l', 'o'};
    std::vector<uint8_t> key2 = {'h', 'e', 'l', 'p'};
    std::vector<uint8_t> key3 = {'w', 'o', 'r', 'l', 'd'};
    
    auto [tree1_1, oldVal1, didUpdate1] = tree1.insert(key1, "greeting");
    auto [tree1_2, oldVal2, didUpdate2] = tree1_1.insert(key2, "assistance");
    auto [tree1_3, oldVal3, didUpdate3] = tree1_2.insert(key3, "planet");
    
    // Look up values
    auto val1 = tree1_3.Get(key1);
    auto val2 = tree1_3.Get(key2);
    auto val3 = tree1_3.Get(key3);
    
    std::cout << "Value for 'hello': " << (val1 ? *val1 : "not found") << std::endl;
    std::cout << "Value for 'help': " << (val2 ? *val2 : "not found") << std::endl;
    std::cout << "Value for 'world': " << (val3 ? *val3 : "not found") << std::endl;
    
    // Delete a key
    auto [tree1_4, oldVal4, found4] = tree1_3.del(key2);
    auto val2_after_delete = tree1_4.Get(key2);
    std::cout << "Value for 'help' after delete: " << (val2_after_delete ? *val2_after_delete : "not found") << std::endl;
    
    // Example 2: Using std::string as key type and int as value type
    std::cout << "\nExample 2: Using std::string as key type and int as value type" << std::endl;
    
    // Create a radix tree with string keys and int values
    RadixTree<std::string, int> tree2;
    
    // Insert some key-value pairs
    auto [tree2_1, oldVal5, didUpdate5] = tree2.insert("apple", 1);
    auto [tree2_2, oldVal6, didUpdate6] = tree2_1.insert("banana", 2);
    auto [tree2_3, oldVal7, didUpdate7] = tree2_2.insert("cherry", 3);
    
    // Look up values
    auto val4 = tree2_3.Get("apple");
    auto val5 = tree2_3.Get("banana");
    auto val6 = tree2_3.Get("cherry");
    
    std::cout << "Value for 'apple': " << (val4 ? std::to_string(*val4) : "not found") << std::endl;
    std::cout << "Value for 'banana': " << (val5 ? std::to_string(*val5) : "not found") << std::endl;
    std::cout << "Value for 'cherry': " << (val6 ? std::to_string(*val6) : "not found") << std::endl;
    
    // Example 3: Using the iterator
    std::cout << "\nExample 3: Using the iterator" << std::endl;
    
    // Create an iterator
    auto it = tree2_3.iterator();
    
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
    auto [tree2_4, oldVal8, didUpdate8] = tree2_3.insert("apricot", 4);
    auto [tree2_5, oldVal9, didUpdate9] = tree2_4.insert("apartment", 5);
    
    // Create an iterator and seek to a prefix
    auto it2 = tree2_5.iterator();
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
    auto rit = ReverseIterator<std::string, int>(tree2_5.getRoot());
    
    // Seek to a lower bound
    rit.seekReverseLowerBound("banana");
    
    // Iterate backwards from the lower bound
    std::cout << "All key-value pairs in reverse order from 'banana':" << std::endl;
    while (true) {
        auto result = rit.previous();
        if (!result.found) break;
        std::cout << "  " << result.key << ": " << result.val << std::endl;
    }

    // Example 6: Testing with words from words.txt
    std::cout << "\nExample 6: Testing RadixTree with words from words.txt" << std::endl;
    
    // Create a radix tree with string keys and int values
    RadixTree<std::string, int> wordTree;
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
    
    for (const auto& word : words) {
        totalWords++;
        auto val = wordTree.Get(word);
        if (val) {
            foundWords++;
            std::cout << word << ": " << *val << std::endl;
        } else {
            std::cout << "ERROR: Word '" << word << "' not found!" << std::endl;
        }
    }
    
    std::cout << "\nSummary:" << std::endl;
    std::cout << "Total words processed: " << totalWords << std::endl;
    std::cout << "Words found in tree: " << foundWords << std::endl;
    std::cout << "Success rate: " << (foundWords * 100.0 / totalWords) << "%" << std::endl;
    
    std::cout << "\nTesting reverse iteration from 'mango':" << std::endl;
    auto wordRit = ReverseIterator<std::string, int>(wordTree.getRoot());
    wordRit.seekReverseLowerBound("mango");
    
    while (true) {
        auto result = wordRit.previous();
        if (!result.found) break;
        std::cout << "  " << result.key << ": " << result.val << std::endl;
    }
    
    return 0;
}
