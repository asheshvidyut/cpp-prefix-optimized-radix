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

// Iterator result structure
template<typename K, typename T>
struct IteratorResult {
    K key;
    T val;
    bool found;
};

// Forward declare Iterator
template<typename K, typename T>
class Iterator;

// ReverseIterator class for traversing nodes in reverse in-order
template<typename K, typename T>
class ReverseIterator {
private:
    std::unique_ptr<Iterator<K, T>> i;
    std::unordered_set<Node<K, T>*> expandedParents;

public:
    ReverseIterator(std::shared_ptr<Node<K, T>> n) : i(std::make_unique<Iterator<K, T>>(n)) {}

    // Seeks the iterator to a given prefix and returns the watch channel
    std::shared_ptr<std::condition_variable> seekPrefixWatch(const K& prefix) {
        return i->seekPrefixWatch(prefix);
    }

    // Seeks the iterator to a given prefix
    void seekPrefix(const K& prefix) {
        i->seekPrefix(prefix);
    }

    // Seeks the iterator to the largest key that is lower or equal to the given key
    void seekReverseLowerBound(const K& key) {
        // Clear the stack
        i->stack.clear();
        auto n = i->node;
        i->node = nullptr;
        K search = key;

        auto found = [this](std::shared_ptr<Node<K, T>> n) {
            std::vector<Edge<K, T>> edges;
            edges.push_back(Edge<K, T>{n->prefix[0], n});
            i->stack.push_back(edges);
            expandedParents.insert(n.get());
        };

        while (n) {
            // Compare current prefix with the search key's same-length prefix
            int prefixCmp;
            if (n->prefix.size() < search.size()) {
                K searchPrefix(search.begin(), search.begin() + n->prefix.size());
                prefixCmp = n->prefix < searchPrefix ? -1 : (n->prefix == searchPrefix ? 0 : 1);
            } else {
                prefixCmp = n->prefix < search ? -1 : (n->prefix == search ? 0 : 1);
            }

            if (prefixCmp < 0) {
                // Prefix is smaller, add to stack and let iterator handle expansion
                std::vector<Edge<K, T>> edges;
                edges.push_back(Edge<K, T>{n->prefix[0], n});
                i->stack.push_back(edges);
                return;
            }

            if (prefixCmp > 0) {
                // Prefix is larger, no reverse lower bound exists
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

                std::vector<Edge<K, T>> edges;
                edges.push_back(Edge<K, T>{n->prefix[0], n});
                i->stack.push_back(edges);
                expandedParents.insert(n.get());
            }

            // Consume the search prefix
            if (n->prefix.size() < search.size()) {
                search = K(search.begin() + n->prefix.size(), search.end());
            } else {
                search.clear();
            }

            if (search.empty()) {
                return;
            }

            // Get lower bound edge
            int idx;
            auto lbNode = n->getLowerBoundEdge(search[0], &idx);

            // Add all edges before the lower bound to the stack
            if (idx == -1) {
                idx = n->edges.size();
            }

            if (idx > 0) {
                i->stack.push_back(std::vector<Edge<K, T>>(n->edges.begin(), n->edges.begin() + idx));
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
        if (i->stack.empty() && i->node) {
            std::vector<Edge<K, T>> edges;
            edges.push_back(Edge<K, T>{i->node->prefix[0], i->node});
            i->stack.push_back(edges);
        }

        while (!i->stack.empty()) {
            // Inspect the last element of the stack
            auto& last = i->stack.back();
            auto elem = last.back().node;

            bool alreadyExpanded = expandedParents.find(elem.get()) != expandedParents.end();

            // Handle internal nodes that haven't been expanded
            if (!elem->edges.empty() && !alreadyExpanded) {
                expandedParents.insert(elem.get());
                i->stack.push_back(elem->edges);
                continue;
            }

            // Remove the node from the stack
            last.pop_back();
            if (last.empty()) {
                i->stack.pop_back();
            }

            if (alreadyExpanded) {
                expandedParents.erase(elem.get());
            }

            // Return leaf node if found
            if (elem->leaf) {
                result.key = elem->leaf->key;
                result.val = elem->leaf->val;
                result.found = true;
                return result;
            }
        }

        return result;
    }
};

// Iterator class
template<typename K, typename T>
class Iterator {
private:
    std::shared_ptr<Node<K, T>> node;
    std::vector<std::vector<Edge<K, T>>> stack;
    K prefix;
    bool patternMatch;
    std::shared_ptr<regex_t> pattern;

    // Helper function to push all edges of a node onto the stack
    void pushEdges(std::shared_ptr<Node<K, T>> n) {
        if (!n || n->edges.empty()) return;
        stack.emplace_back(n->edges);
    }

    // Helper function to check if one sequence is a prefix of another
    static bool hasPrefix(const K& str, const K& prefix) {
        if (str.size() < prefix.size()) return false;
        return std::equal(prefix.begin(), prefix.end(), str.begin());
    }

public:
    Iterator(std::shared_ptr<Node<K, T>> n) : node(n), patternMatch(false) {
        if (node) {
            pushEdges(node);
        }
    }

    // Sets pattern matching for the iterator
    void setPatternMatch(regex_t* regex) {
        patternMatch = true;
        pattern = std::shared_ptr<regex_t>(regex, [](regex_t* p) { /* No-op deleter */ });
    }

    // Seeks the iterator to a given prefix and returns the watch channel
    std::shared_ptr<std::condition_variable> seekPrefixWatch(const K& prefix) {
        // Clear the stack
        stack.clear();
        auto n = node;
        node = nullptr;
        K search = prefix;
        std::shared_ptr<std::condition_variable> watch = nullptr;

        while (n) {
            // Compare current prefix with the search key's same-length prefix
            int prefixCmp;
            if (n->prefix.size() < search.size()) {
                K searchPrefix(search.begin(), search.begin() + n->prefix.size());
                prefixCmp = n->prefix < searchPrefix ? -1 : (n->prefix == searchPrefix ? 0 : 1);
            } else {
                prefixCmp = n->prefix < search ? -1 : (n->prefix == search ? 0 : 1);
            }

            if (prefixCmp < 0) {
                // Prefix is smaller, add to stack and let iterator handle expansion
                std::vector<Edge<K, T>> edges;
                edges.push_back(Edge<K, T>{n->prefix[0], n});
                stack.push_back(edges);
                return n->mutateCh;
            }

            if (prefixCmp > 0) {
                // Prefix is larger, no match exists
                return nullptr;
            }

            // Update watch to the most specific node
            watch = n->mutateCh;

            // Handle leaf nodes
            if (n->leaf) {
                if (hasPrefix(n->leaf->key, prefix)) {
                    std::vector<Edge<K, T>> edges;
                    edges.push_back(Edge<K, T>{n->prefix[0], n});
                    stack.push_back(edges);
                    return watch;
                }
                return nullptr;
            }

            // Consume the search prefix
            if (n->prefix.size() < search.size()) {
                search = K(search.begin() + n->prefix.size(), search.end());
            } else {
                search.clear();
            }

            if (search.empty()) {
                std::vector<Edge<K, T>> edges;
                edges.push_back(Edge<K, T>{n->prefix[0], n});
                stack.push_back(edges);
                return watch;
            }

            // Get the edge for the next character
            auto nextNode = n->getEdge(search[0]);
            if (!nextNode) {
                return watch;
            }

            n = nextNode;
        }

        return watch;
    }

    // Seeks the iterator to a given prefix
    void seekPrefix(const K& prefix) {
        seekPrefixWatch(prefix);
    }

    // Seeks the iterator to the smallest key that is greater or equal to the given key
    void seekLowerBound(const K& key) {
        // Clear the stack
        stack.clear();
        auto n = node;
        node = nullptr;
        K search = key;

        auto found = [this](std::shared_ptr<Node<K, T>> n) {
            std::vector<Edge<K, T>> edges;
            edges.push_back(Edge<K, T>{n->prefix[0], n});
            stack.push_back(edges);
        };

        while (n) {
            // Compare current prefix with the search key's same-length prefix
            int prefixCmp;
            if (n->prefix.size() < search.size()) {
                K searchPrefix(search.begin(), search.begin() + n->prefix.size());
                prefixCmp = n->prefix < searchPrefix ? -1 : (n->prefix == searchPrefix ? 0 : 1);
            } else {
                prefixCmp = n->prefix < search ? -1 : (n->prefix == search ? 0 : 1);
            }

            if (prefixCmp < 0) {
                // Prefix is smaller, add to stack and let iterator handle expansion
                found(n);
                return;
            }

            if (prefixCmp > 0) {
                // Prefix is larger, no lower bound exists
                return;
            }

            // Handle leaf nodes
            if (n->leaf) {
                if (n->leaf->key >= key) {
                    found(n);
                    return;
                }

                // If this is the last leaf in this subtree, we need to check the next subtree
                if (n->edges.empty()) {
                    found(n);
                    return;
                }

                // Otherwise, continue searching in child nodes
                found(n);
                return;
            }

            // Consume the search prefix
            if (n->prefix.size() < search.size()) {
                search = K(search.begin() + n->prefix.size(), search.end());
            } else {
                search.clear();
            }

            if (search.empty()) {
                // We've matched the prefix, now find the smallest key in this subtree
                found(n);
                return;
            }

            // Get lower bound edge
            int idx;
            auto lbNode = n->getLowerBoundEdge(search[0], &idx);

            // Add all edges before the lower bound to the stack in reverse order
            // This ensures we process them in ascending order
            if (idx == -1) {
                idx = n->edges.size();
            }

            if (idx > 0) {
                std::vector<Edge<K, T>> edges;
                for (int i = idx - 1; i >= 0; i--) {
                    edges.push_back(n->edges[i]);
                }
                stack.push_back(edges);
            }

            if (!lbNode) {
                return;
            }

            n = lbNode;
        }
    }

    // Returns the next node in order
    IteratorResult<K, T> next() {
        IteratorResult<K, T> result;
        result.found = false;

        // Initialize stack if needed
        if (stack.empty() && node) {
            std::vector<Edge<K, T>> edges;
            edges.push_back(Edge<K, T>{node->prefix[0], node});
            stack.push_back(edges);
        }

        while (!stack.empty()) {
            // Inspect the last element of the stack
            auto& last = stack.back();
            if (last.empty()) {
                stack.pop_back();
                continue;
            }

            // Get the first edge from the current level
            auto elem = last.front().node;
            last.erase(last.begin());

            // Handle internal nodes
            if (!elem->edges.empty()) {
                stack.push_back(elem->edges);
                continue;
            }

            // Return leaf node if found
            if (elem->leaf) {
                if (patternMatch) {
                    if (regexec(pattern.get(), elem->leaf->key.c_str(), 0, nullptr, 0) != 0) {
                        continue;
                    }
                }
                result.key = elem->leaf->key;
                result.val = elem->leaf->val;
                result.found = true;
                return result;
            }
        }

        return result;
    }

    friend class ReverseIterator<K, T>;
};

// Helper function to create an iterator from a node
template<typename K, typename T>
Iterator<K, T> createIterator(std::shared_ptr<Node<K, T>> node) {
    return Iterator<K, T>(node);
}

#endif // ITERATOR_H