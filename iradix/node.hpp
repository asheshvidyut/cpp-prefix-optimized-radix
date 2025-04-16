//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef NODE_H
#define NODE_H

#include <regex.h>  // if you use regex_t for patterns
#include <vector>
#include <memory>
#include <algorithm>
#include <cstring>
#include <condition_variable>

// Forward declarations
template<typename K, typename T>
class Node;

template<typename K, typename T>
class LeafNode;

// Edge structure
template<typename K, typename T>
struct Edge {
    typename K::value_type label;
    std::shared_ptr<Node<K, T>> node;
};

// LeafNode definition
template<typename K, typename T>
class LeafNode {
public:
    K key;                  // key data
    T val;                  // value
    std::shared_ptr<LeafNode<K, T>> nextLeaf;
    std::shared_ptr<LeafNode<K, T>> prevLeaf;

    LeafNode(const K& k, const T& v) : key(k), val(v), nextLeaf(nullptr), prevLeaf(nullptr) {}
};

// Node structure
template<typename K, typename T>
class Node {
public:
    std::shared_ptr<LeafNode<K, T>> leaf;
    std::shared_ptr<LeafNode<K, T>> minLeaf;
    std::shared_ptr<LeafNode<K, T>> maxLeaf;
    K prefix;
    std::vector<Edge<K, T>> edges;
    std::shared_ptr<std::condition_variable> mutateCh;

    Node() : leaf(nullptr), minLeaf(nullptr), maxLeaf(nullptr), 
             mutateCh(std::make_shared<std::condition_variable>()) {}

    // Returns the edge with the given label, or nullptr if not found
    std::shared_ptr<Node<K, T>> getEdge(typename K::value_type label, int* out_index = nullptr) const {
        if (edges.empty()) {
            if (out_index) *out_index = -1;
            return nullptr;
        }

        auto it = std::find_if(edges.begin(), edges.end(), 
            [label](const Edge<K, T>& e) { return e.label == label; });
        
        if (it != edges.end()) {
            if (out_index) *out_index = std::distance(edges.begin(), it);
            return it->node;
        }
        
        if (out_index) *out_index = -1;
        return nullptr;
    }

    // Replaces an edge with the given label
    void replaceEdge(const Edge<K, T>& e) {
        auto it = std::find_if(edges.begin(), edges.end(), 
            [&e](const Edge<K, T>& edge) { return edge.label == e.label; });
        
        if (it != edges.end()) {
            *it = e;
        } else {
            addEdge(e);
        }
    }

    // Deletes an edge with the given label
    void delEdge(typename K::value_type label) {
        auto it = std::find_if(edges.begin(), edges.end(), 
            [label](const Edge<K, T>& e) { return e.label == label; });
        
        if (it != edges.end()) {
            edges.erase(it);
        }
    }

    // Adds an edge in sorted order
    void addEdge(const Edge<K, T>& e) {
        auto it = std::lower_bound(edges.begin(), edges.end(), e,
            [](const Edge<K, T>& a, const Edge<K, T>& b) { return a.label < b.label; });
        
        edges.insert(it, e);
    }

    // Returns the lower bound edge for the given label
    std::shared_ptr<Node<K, T>> getLowerBoundEdge(typename K::value_type label, int* out_index) const {
        if (edges.empty()) {
            if (out_index) *out_index = -1;
            return nullptr;
        }

        auto it = std::lower_bound(edges.begin(), edges.end(), label,
            [](const Edge<K, T>& e, typename K::value_type l) { return e.label < l; });
        
        if (out_index) *out_index = std::distance(edges.begin(), it);
        
        if (it != edges.end()) {
            return it->node;
        }
        
        return nullptr;
    }

    // Returns the minimum leaf in the node
    std::shared_ptr<LeafNode<K, T>> minimumLeaf(bool* found = nullptr) const {
        if (leaf) {
            if (found) *found = true;
            return leaf;
        }
        
        if (edges.empty()) {
            if (found) *found = false;
            return nullptr;
        }
        
        return edges[0].node->minimumLeaf(found);
    }

    // Returns the maximum leaf in the node
    std::shared_ptr<LeafNode<K, T>> maximumLeaf(bool* found = nullptr) const {
        if (leaf) {
            if (found) *found = true;
            return leaf;
        }
        
        if (edges.empty()) {
            if (found) *found = false;
            return nullptr;
        }
        
        return edges.back().node->maximumLeaf(found);
    }

    // Updates the node's minLeaf and maxLeaf fields
    void updateMinMaxLeaves() {
        if (leaf) {
            minLeaf = leaf;
            maxLeaf = leaf;
            return;
        }
        
        if (edges.empty()) {
            minLeaf = nullptr;
            maxLeaf = nullptr;
            return;
        }
        
        bool found;
        minLeaf = edges[0].node->minimumLeaf(&found);
        if (!found) minLeaf = nullptr;
        
        maxLeaf = edges.back().node->maximumLeaf(&found);
        if (!found) maxLeaf = nullptr;
    }

    // Checks if the node is a leaf
    bool isLeaf() const {
        return leaf != nullptr;
    }
};

// Result structure for LongestPrefix
template<typename K, typename T>
struct LongestPrefixResult {
    K key;
    T val;
    bool found;
};

// Helper function to find the longest common prefix between two sequences
template<typename K>
int longestPrefix(const K& k1, const K& k2) {
    auto it1 = k1.begin();
    auto it2 = k2.begin();
    int count = 0;
    
    while (it1 != k1.end() && it2 != k2.end() && *it1 == *it2) {
        ++it1;
        ++it2;
        ++count;
    }
    
    return count;
}

// Helper function to concatenate two sequences
template<typename K>
K concat(const K& a, const K& b) {
    K result;
    result.reserve(a.size() + b.size());
    result.insert(result.end(), a.begin(), a.end());
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

#endif // NODE_H
