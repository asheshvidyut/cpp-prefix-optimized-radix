//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef ITERATOR_H
#define ITERATOR_H

#include "node.hpp"
#include <vector>
#include <memory>
#include <regex.h>
#include <optional>
#include <unordered_set>

// Forward declarations
template<typename K, typename T>
class Node;

template<typename K, typename T>
class Iterator;

// Iterator result structure
template<typename K, typename T>
struct IteratorResult {
    K key;
    T val;
    bool found;
};

// Iterator class
template<typename K, typename T>
class Iterator {
private:
    std::shared_ptr<Node<K, T>> node;
    std::vector<std::pair<std::shared_ptr<Node<K, T>>, size_t>> stack;  // Node and edge index
    K prefix;
    bool patternMatch;
    std::shared_ptr<regex_t> pattern;

    // Helper function to push all edges of a node onto the stack
    void pushEdges(std::shared_ptr<Node<K, T>> n) {
        if (!n || n->edges.empty()) return;
        stack.emplace_back(n, 0);
    }

public:
    explicit Iterator(std::shared_ptr<Node<K, T>> root) 
        : node(root), patternMatch(false) {
        if (node) {
            pushEdges(node);
        }
    }

    // Sets pattern matching for the iterator
    void setPatternMatch(regex_t* regex) {
        patternMatch = true;
        pattern = std::shared_ptr<regex_t>(regex, [](regex_t* p) { /* No-op deleter */ });
    }

    // Seeks the iterator to a given prefix
    void seekPrefix(const K& prefix) {
        // Reset state
        this->prefix = prefix;
        stack.clear();
        
        if (prefix.empty()) {
            // Handle empty prefix case - start from root
            pushEdges(node);
            return;
        }

        // Start from root and traverse to the prefix
        auto current = node;
        K search = prefix;

        while (current && !search.empty()) {
            // Look for an edge
            auto child = current->getEdge(search[0]);
            if (!child) {
                node = nullptr;
                return;
            }

            // Check if the child's prefix matches our search
            if (hasPrefix(search, child->prefix)) {
                search.erase(search.begin(), search.begin() + child->prefix.size());
                current = child;
            } else if (hasPrefix(child->prefix, search)) {
                // The search is a prefix of the child's prefix
                node = child;
                pushEdges(child);
                return;
            } else {
                node = nullptr;
                return;
            }
        }

        node = current;
        pushEdges(current);
    }

    // Returns the next element from the iterator
    IteratorResult<K, T> next() {
        IteratorResult<K, T> result;
        result.found = false;

        // Check if we have a current node with a leaf
        if (node && node->leaf) {
            // If we have a prefix, make sure the leaf matches it
            if (prefix.empty() || hasPrefix(node->leaf->key, prefix)) {
                result.key = node->leaf->key;
                result.val = node->leaf->val;
                result.found = true;
            }
            node = nullptr;  // Move past this leaf
            return result;
        }

        // Process the stack
        while (!stack.empty()) {
            auto& [current, edgeIdx] = stack.back();
            
            // If we've processed all edges of the current node, pop it
            if (edgeIdx >= current->edges.size()) {
                stack.pop_back();
                continue;
            }

            // Get the next edge
            auto edge = current->edges[edgeIdx++];
            auto next = edge.node;

            // If this is a leaf node, return its value
            if (next->leaf) {
                if (prefix.empty() || hasPrefix(next->leaf->key, prefix)) {
                    result.key = next->leaf->key;
                    result.val = next->leaf->val;
                    result.found = true;
                    
                    // If there are more edges, push them onto the stack
                    if (!next->edges.empty()) {
                        pushEdges(next);
                    }
                    
                    return result;
                }
            }

            // Push the next node's edges onto the stack
            pushEdges(next);
        }

        return result;
    }

private:
    // Helper function to check if one sequence is a prefix of another
    static bool hasPrefix(const K& str, const K& prefix) {
        if (str.size() < prefix.size()) return false;
        return std::equal(prefix.begin(), prefix.end(), str.begin());
    }
};

// Helper function to create an iterator from a node
template<typename K, typename T>
Iterator<K, T> createIterator(std::shared_ptr<Node<K, T>> node) {
    return Iterator<K, T>(node);
}

// ReverseIterator class for traversing nodes in reverse in-order
template<typename K, typename T>
class ReverseIterator {
private:
    std::shared_ptr<Iterator<K, T>> iter;
    std::unordered_set<std::shared_ptr<Node<K, T>>> expandedParents;

public:
    explicit ReverseIterator(std::shared_ptr<Node<K, T>> root) 
        : iter(std::make_shared<Iterator<K, T>>(root)) {}

    // Seeks the iterator to a given prefix and returns the watch channel
    std::shared_ptr<std::condition_variable> seekPrefixWatch(const K& prefix) {
        return iter->seekPrefixWatch(prefix);
    }

    // Seeks the iterator to a given prefix
    void seekPrefix(const K& prefix) {
        iter->seekPrefixWatch(prefix);
    }

    // Seeks the iterator to the largest key that is lower or equal to the given key
    void seekReverseLowerBound(const K& key) {
        // Wipe the stack
        iter->stack.clear();
        auto n = iter->node;
        iter->node = nullptr;
        K search = key;

        auto found = [this](std::shared_ptr<Node<K, T>> n) {
            std::vector<Edge<K, T>> single_edge;
            single_edge.push_back(Edge<K, T>{n->prefix[0], n});
            iter->stack.push_back(single_edge);
            expandedParents.insert(n);
        };

        while (n) {
            // Compare current prefix with the search key's same-length prefix
            int prefixCmp;
            if (n->prefix.size() < search.size()) {
                K searchPrefix(search.begin(), search.begin() + n->prefix.size());
                prefixCmp = compareKeys(n->prefix, searchPrefix);
            } else {
                prefixCmp = compareKeys(n->prefix, search);
            }

            if (prefixCmp < 0) {
                // Prefix is smaller, add to stack and let iterator handle expansion
                std::vector<Edge<K, T>> single_edge;
                single_edge.push_back(Edge<K, T>{n->prefix[0], n});
                iter->stack.push_back(single_edge);
                return;
            }

            if (prefixCmp > 0) {
                // No reverse lower bound exists
                return;
            }

            // Handle leaf nodes
            if (n->leaf) {
                if (n->leaf->key == key) {
                    found(n);
                    return;
                }

                if (n->edges.empty()) {
                    found(n);
                    return;
                }

                std::vector<Edge<K, T>> single_edge;
                single_edge.push_back(Edge<K, T>{n->prefix[0], n});
                iter->stack.push_back(single_edge);
                expandedParents.insert(n);
            }

            // Consume the search prefix
            if (!n->prefix.empty() && !search.empty()) {
                search.erase(search.begin(), search.begin() + std::min(n->prefix.size(), search.size()));
            }

            if (search.empty()) {
                return;
            }

            // Get lower bound edge
            int idx;
            auto lbNode = n->getLowerBoundEdge(search[0], &idx);

            // Add lower edges to stack
            if (idx == -1) {
                idx = n->edges.size();
            }

            if (idx > 0) {
                std::vector<Edge<K, T>> edges(n->edges.begin(), n->edges.begin() + idx);
                iter->stack.push_back(edges);
            }

            if (!lbNode) {
                return;
            }

            n = lbNode;
        }
    }

    // Returns the previous element in reverse order
    IteratorResult<K, T> previous() {
        IteratorResult<K, T> result;
        result.found = false;

        // Initialize stack if needed
        if (iter->stack.empty() && iter->node) {
            std::vector<Edge<K, T>> single_edge;
            single_edge.push_back(Edge<K, T>{iter->node->prefix[0], iter->node});
            iter->stack.push_back(single_edge);
        }

        while (!iter->stack.empty()) {
            // Inspect the last element of the stack
            auto& edges = iter->stack.back();
            if (edges.empty()) {
                iter->stack.pop_back();
                continue;
            }

            auto edge = edges.back();
            auto elem = edge.node;

            auto it = expandedParents.find(elem);
            bool alreadyExpanded = (it != expandedParents.end());

            // Handle internal nodes that haven't been expanded
            if (!elem->edges.empty() && !alreadyExpanded) {
                expandedParents.insert(elem);
                iter->stack.push_back(elem->edges);
                continue;
            }

            // Remove the node from the stack
            edges.pop_back();
            if (edges.empty()) {
                iter->stack.pop_back();
            }

            if (alreadyExpanded) {
                expandedParents.erase(elem);
            }

            // Return leaf value if present
            if (elem->leaf) {
                result.key = elem->leaf->key;
                result.val = elem->leaf->val;
                result.found = true;
                return result;
            }
        }

        return result;
    }

private:
    // Helper function to compare two sequences
    static int compareKeys(const K& a, const K& b) {
        auto it1 = a.begin();
        auto it2 = b.begin();
        
        while (it1 != a.end() && it2 != b.end()) {
            if (*it1 < *it2) return -1;
            if (*it1 > *it2) return 1;
            ++it1;
            ++it2;
        }
        
        if (it1 == a.end() && it2 == b.end()) return 0;
        if (it1 == a.end()) return -1;
        return 1;
    }
};

#endif // ITERATOR_H