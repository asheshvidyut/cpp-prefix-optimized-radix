//
// Created by Ashesh Vidyut on 22/03/25.
//

#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <algorithm>
#include <random>

#include "radix.hpp"
#include "iterator.hpp"


int main() {
    // Open the file "words.txt"
    std::ifstream file("/Users/asheshvidyut/rsquare/radix/words.txt");
    if (!file.is_open()) {
        std::cerr << "Error opening file: words.txt" << std::endl;
        return 1;
    }

    // Read the file line by line
    std::vector<std::string> lines;
    std::string line;
    while (std::getline(file, line)) {
        lines.push_back(line);
    }
    file.close();

    // Shuffle lines using std::shuffle and a random engine seeded with current time.
    unsigned seed = std::chrono::system_clock::now().time_since_epoch().count();
//    std::shuffle(lines.begin(), lines.end(), std::default_random_engine(seed));

    // Create a new radix tree. Assume New() returns a pointer to a Tree.
    Tree* r = Tree_New();

    // Insert each line into the tree.
    // Assume Insert takes key as unsigned char*, its length, and a value (here NULLptr),
    // and returns a new Tree pointer (ignoring old value and update flag).
    for (const auto &s: lines) {
        void *oldVal = NULL;
        int didUpdate = 0;
        r = Tree_Insert(r, (unsigned char*)s.c_str(), s.size(), NULL, &oldVal, &didUpdate);
    }

    // Get an iterator from the tree's root.
    // Assume Root() returns a pointer to a Node, and that Node has a method Iterator().
    Iterator* iter = Node_Iterator(Tree_Root(r));
    iter->key = (unsigned char*)"";
    iter->key_len = 0;

    // Iterate over the tree, collecting keys into the result vector.
    std::vector<std::string> result;
    int counter = 0;
    while (true) {
        // Assume Iterator::Next() returns a tuple: (std::string key, void* value, bool found)
        auto iteratorResult = Iterator_Next(iter);
        if (!iteratorResult.found)
            break;
        result.push_back(std::string(reinterpret_cast<char*>(iteratorResult.key), iteratorResult.key_len));
        counter++;
    }

    // Validate that the number of items returned equals the number of lines.
    if (static_cast<size_t>(counter) != lines.size()) {
        std::cerr << "Test failed: expected " << lines.size() << ", got " << counter << std::endl;
        return 1;
    } else {
        std::cout << "Test passed: " << counter << " items found" << std::endl;
    }

    // Optionally: free tree, iterator, and any other allocated resources as needed.

    // Test seekPrefix
    iter = Node_Iterator(Tree_Root(r));
    auto * pfx = (unsigned char *)"b";
    Iterator_SeekPrefix(iter, pfx, 1);


    while (true) {
        // Assume Iterator::Next() returns a tuple: (std::string key, void* value, bool found)
        auto iteratorResult = Iterator_Next(iter);
        if (!iteratorResult.found)
            break;
        printf("%s\n", std::string(reinterpret_cast<char*>(iteratorResult.key), iteratorResult.key_len).c_str());
    }

    r = Tree_New();
    char * s = "a";
    int oldVal;
    int didUpdate;
    r = Tree_Insert(r, (unsigned char*)s, 1, NULL, reinterpret_cast<void **>(&oldVal), &didUpdate);
    s = "ab";
    r = Tree_Insert(r, (unsigned char*)s, 2, NULL, reinterpret_cast<void **>(&oldVal), &didUpdate);
    s = "abc";
    r = Tree_Insert(r, (unsigned char*)s, 3, NULL, reinterpret_cast<void **>(&oldVal), &didUpdate);
    iter = Node_Iterator(Tree_Root(r));
    iter->key_len = 0;
    iter->key = (unsigned char *)"";
    while (true) {
        // Assume Iterator::Next() returns a tuple: (std::string key, void* value, bool found)
        auto iteratorResult = Iterator_Next(iter);
        if (!iteratorResult.found)
            break;
        printf("%s\n", std::string(reinterpret_cast<char*>(iteratorResult.key), iteratorResult.key_len).c_str());
    }

    return 0;
}
