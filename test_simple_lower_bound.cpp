#include <iostream>
#include <string>
#include "radix/tree.hpp"

int main() {
    std::cout << "Testing LowerBoundIterator basic functionality..." << std::endl;
    
    Tree<std::string, std::string> tree;
    
    // Insert some test data
    tree.insert("a", "alpha");
    tree.insert("ab", "beta");
    tree.insert("abc", "gamma");
    tree.insert("b", "bravo");
    tree.insert("c", "charlie");
    
    std::cout << "Tree size: " << tree.len() << std::endl;
    
    // Test lower bound iterator
    auto lbIter = tree.lowerBoundIterator("abb");
    lbIter.seekLowerBound("abb");
    
    std::cout << "Results from lower bound 'abb':" << std::endl;
    while (true) {
        auto result = lbIter.next();
        if (!result.found) break;
        std::cout << "  " << result.key << " -> " << result.val << std::endl;
    }
    
    std::cout << "Test completed!" << std::endl;
    return 0;
} 