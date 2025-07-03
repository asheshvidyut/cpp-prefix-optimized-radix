#include "radix/tree.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

void testReverseIteratorSeekLowerBound() {
    std::cout << "=== Testing ReverseIterator SeekLowerBound ===" << std::endl;
    
    // Fixed length keys (should be defined in order)
    std::vector<std::string> fixedLenKeys = {
        "20020",
        "00020", 
        "00010",
        "00004",
        "00001",
        "00000"
    };
    
    // Mixed length keys (should be defined in order)
    std::vector<std::string> mixedLenKeys = {
        "zip",
        "zap", 
        "found",
        "foo",
        "f",
        "barbazboo",
        "abc",
        "a1"
    };
    
    // Test cases: {keys, search, expected_result}
    std::vector<std::tuple<std::vector<std::string>, std::string, std::vector<std::string>>> testCases = {
        {
            fixedLenKeys,
            "20020",
            fixedLenKeys
        },
        {
            fixedLenKeys,
            "20000",
            {"00020", "00010", "00004", "00001", "00000"}
        },
        {
            fixedLenKeys,
            "00010",
            {"00010", "00004", "00001", "00000"}
        },
        {
            fixedLenKeys,
            "00000",
            {"00000"}
        },
        {
            fixedLenKeys,
            "0",
            {}
        },
        {
            mixedLenKeys,
            "{", // after all lower case letters
            mixedLenKeys
        },
        {
            mixedLenKeys,
            "zip",
            mixedLenKeys
        },
        {
            mixedLenKeys,
            "b",
            {"abc", "a1"}
        },
        {
            mixedLenKeys,
            "barbazboo0",
            {"barbazboo", "abc", "a1"}
        },
        {
            mixedLenKeys,
            "a",
            {}
        },
        {
            mixedLenKeys,
            "a1",
            {"a1"}
        },
        // Test prefix keys
        {
            {"f", "fo", "foo", "food", "bug"},
            "foo",
            {"foo", "fo", "f", "bug"}
        },
        {
            {"f", "fo", "foo", "food", "bug"},
            "foozzzzzzzzzz", // larger than any key but with shared prefix
            {"food", "foo", "fo", "f", "bug"}
        },
        // Test empty key
        {
            {"f", "fo", "foo", "food", "bug", ""},
            "foo",
            {"foo", "fo", "f", "bug", ""}
        },
        {
            {"f", "bug", ""},
            "",
            {""}
        },
        {
            {"f", "bug", "xylophone"},
            "",
            {}
        },
        // Edge cases
        {
            {"foo00", "foo11"},
            "foo",
            {}
        },
        {
            {"bar", "foo00", "foo11"},
            "foo",
            {"bar"}
        },
        {
            {"bdgedcdc", "agcbcaba"},
            "beefdafg",
            {"bdgedcdc", "agcbcaba"}
        }
    };
    
    for (size_t idx = 0; idx < testCases.size(); idx++) {
        auto [keys, search, expected] = testCases[idx];
        
        std::cout << "Test case " << idx << ": search='" << search << "'" << std::endl;
        
        // Create tree and insert keys
        Tree<std::string, std::string> tree;
        for (const auto& key : keys) {
            auto [newTree, oldVal, didUpdate] = tree.insert(key, key);
            tree = newTree;
        }
        
        if (tree.len() != keys.size()) {
            std::cout << "ERROR: Failed adding keys. Expected " << keys.size() 
                      << ", got " << tree.len() << std::endl;
            continue;
        }
        
        // Get reverse iterator and seek to lower bound
        auto iter = ReverseIterator<std::string, std::string>(tree.getRoot());
        iter.seekLowerBound(search);
        
        // Collect all keys
        std::vector<std::string> result;
        while (true) {
            auto res = iter.previous();
            if (!res.found) break;
            result.push_back(res.key);
        }
        
        // Compare results
        if (result != expected) {
            std::cout << "MISMATCH: search='" << search << "'" << std::endl;
            std::cout << "  Expected: ";
            for (const auto& k : expected) std::cout << "'" << k << "' ";
            std::cout << std::endl;
            std::cout << "  Got:      ";
            for (const auto& k : result) std::cout << "'" << k << "' ";
            std::cout << std::endl;
        } else {
            std::cout << "  PASS" << std::endl;
        }
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
    testReverseIteratorSeekLowerBound();
    testReverseIteratorSeekPrefix();
    testReverseIteratorPrevious();
    
    std::cout << "\n=== Reverse Iterator Tests Complete ===" << std::endl;
    return 0;
} 