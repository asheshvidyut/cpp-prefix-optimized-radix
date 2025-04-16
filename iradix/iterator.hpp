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
    std::vector<std::vector<Edge<K, T>>> stack;
    std::shared_ptr<LeafNode<K, T>> leafNode;
    K key;
    bool patternMatch;
    std::shared_ptr<regex_t> pattern;

    // Helper function to find minimum node recursively
    std::shared_ptr<Node<K, T>> recurseMin(std::shared_ptr<Node<K, T>> n) {
        if (n->leaf) {
            return n;
        }
        
        size_t nEdges = n->edges.size();
        if (nEdges > 1) {
            // Add all the other edges to the stack (the min node will be added as
            // we recurse)
            stack.push_back(std::vector<Edge<K, T>>(n->edges.begin() + 1, n->edges.end()));
        }
        
        if (nEdges > 0) {
            return recurseMin(n->edges[0].node);
        }
        
        // Shouldn't be possible
        return nullptr;
    }

public:
    explicit Iterator(std::shared_ptr<Node<K, T>> root) 
        : node(root), leafNode(nullptr), patternMatch(false) {
        if (node) {
            node->mutateCh = std::make_shared<std::condition_variable>();
        }
    }

    // Sets pattern matching for the iterator
    void setPatternMatch(regex_t* regex) {
        patternMatch = true;
        pattern = std::shared_ptr<regex_t>(regex, [](regex_t* p) { /* No-op deleter */ });
    }

    // Seeks the iterator to a given prefix (watch variant)
    std::shared_ptr<std::condition_variable> seekPrefixWatch(const K& prefix) {
        // Wipe the stack
        stack.clear();
        std::shared_ptr<Node<K, T>> n = node;
        auto watch = n->mutateCh;
        K search = prefix;

        while (true) {
            // Check for key exhaustion
            if (search.empty()) {
                node = n;
                return watch;
            }

            // Look for an edge
            n = n->getEdge(search[0]);
            if (!n) {
                node = nullptr;
                return watch;
            }

            // Update to the finest granularity as the search makes progress
            watch = n->mutateCh;

            // Consume the search prefix
            if (hasPrefix(search, n->prefix)) {
                search.erase(search.begin(), search.begin() + n->prefix.size());
            } else if (hasPrefix(n->prefix, search)) {
                node = n;
                return watch;
            } else {
                node = nullptr;
                return watch;
            }
        }
    }

    // Seeks the iterator to a given prefix
    void seekPrefix(const K& prefix) {
        if (prefix.empty()) {
            // Handle empty prefix case
            stack.clear();
            if (node) {
                std::vector<Edge<K, T>> edges;
                for (const auto& edge : node->edges) {
                    edges.push_back(edge);
                }
                stack.push_back(edges);
            }
            return;
        }
        seekPrefixWatch(prefix);
    }

    // Seeks the iterator to the smallest key that is greater or equal to the given key
    void seekLowerBound(const K& key) {
        // Wipe the stack
        stack.clear();
        std::shared_ptr<Node<K, T>> n = node;
        node = nullptr;
        K search = key;

        auto found = [this](std::shared_ptr<Node<K, T>> n) {
            std::vector<Edge<K, T>> single_edge;
            single_edge.push_back(Edge<K, T>{n->prefix[0], n});
            stack.push_back(single_edge);
        };

        auto findMin = [this, &found](std::shared_ptr<Node<K, T>> n) {
            n = recurseMin(n);
            if (n) {
                found(n);
            }
        };

        while (true) {
            // Compare current prefix with the search key's same-length prefix
            int prefixCmp;
            if (n->prefix.size() < search.size()) {
                prefixCmp = compareKeys(n->prefix, 
                    K(search.begin(), search.begin() + n->prefix.size()));
            } else {
                prefixCmp = compareKeys(n->prefix, search);
            }

            if (prefixCmp > 0) {
                findMin(n);
                return;
            }

            if (prefixCmp < 0) {
                node = nullptr;
                return;
            }

            // Check for exact match at leaf
            if (n->leaf && n->leaf->key == key) {
                found(n);
                return;
            }

            // Consume the search prefix
            search.erase(search.begin(), search.begin() + n->prefix.size());

            if (search.empty()) {
                findMin(n);
                return;
            }

            // Get lower bound edge
            int idx;
            auto lbNode = n->getLowerBoundEdge(search[0], &idx);
            if (!lbNode) {
                return;
            }

            // Add higher edges to stack
            if (idx + 1 < n->edges.size()) {
                stack.push_back(std::vector<Edge<K, T>>(
                    n->edges.begin() + idx + 1, n->edges.end()));
            }

            n = lbNode;
        }
    }

    // Returns the next element from the iterator
    IteratorResult<K, T> next() {
        IteratorResult<K, T> result;
        result.found = false;

        // If we have a current leaf node, try to get the next one
        if (leafNode) {
            if (leafNode->nextLeaf) {
                leafNode = leafNode->nextLeaf;
                result.key = leafNode->key;
                result.val = leafNode->val;
                result.found = true;
                return result;
            }
            leafNode = nullptr;
            return result;
        }

        // If we don't have a current node, start from the root
        if (!node) {
            return result;
        }

        // Find the leftmost leaf node
        std::shared_ptr<Node<K, T>> current = node;
        while (current) {
            if (current->leaf) {
                leafNode = current->leaf;
                result.key = leafNode->key;
                result.val = leafNode->val;
                result.found = true;
                return result;
            }

            if (current->edges.empty()) {
                break;
            }

            current = current->edges[0].node;
        }

        return result;
    }

private:
    // Helper function to check if one sequence is a prefix of another
    static bool hasPrefix(const K& str, const K& prefix) {
        if (str.size() < prefix.size()) return false;
        return std::equal(prefix.begin(), prefix.end(), str.begin());
    }

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