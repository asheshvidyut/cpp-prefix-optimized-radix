//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "node.hpp"
#include <algorithm>
#include <cstring>

// This file is intentionally left mostly empty as the implementation is now in the header file.
// The template-based implementation is defined in node.hpp.

// If you need to add any non-template functions or implementations that can't be in the header,
// you can add them here.

// Example of a non-template helper function that might be needed:
void initializeNode() {
    // Any global initialization code for nodes
}

// Helper function implementations
template<typename K>
bool hasPrefix(const K& str, const K& prefix) {
    if (prefix.size() > str.size()) {
        return false;
    }
    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

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

template<typename K>
K concat(const K& a, const K& b) {
    K result;
    result.reserve(a.size() + b.size());
    result.insert(result.end(), a.begin(), a.end());
    result.insert(result.end(), b.begin(), b.end());
    return result;
}

// LeafNode implementation
template<typename K, typename T>
LeafNode<K, T>::LeafNode(const K& k, const T& v) : key(k), val(v), nextLeaf(nullptr), prevLeaf(nullptr) {}

// Node implementation
template<typename K, typename T>
Node<K, T>::Node() : leaf(nullptr), minLeaf(nullptr), maxLeaf(nullptr), 
                     mutateCh(std::make_shared<std::condition_variable>()) {}

template<typename K, typename T>
std::shared_ptr<Node<K, T>> Node<K, T>::getEdge(typename K::value_type label, int* out_index) const {
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

template<typename K, typename T>
void Node<K, T>::replaceEdge(const Edge<K, T>& e) {
    auto it = std::find_if(edges.begin(), edges.end(), 
        [&e](const Edge<K, T>& edge) { return edge.label == e.label; });
    
    if (it != edges.end()) {
        *it = e;
    } else {
        addEdge(e);
    }
}

template<typename K, typename T>
void Node<K, T>::delEdge(typename K::value_type label) {
    auto it = std::find_if(edges.begin(), edges.end(), 
        [label](const Edge<K, T>& e) { return e.label == label; });
    
    if (it != edges.end()) {
        edges.erase(it);
    }
}

template<typename K, typename T>
void Node<K, T>::addEdge(const Edge<K, T>& e) {
    auto it = std::lower_bound(edges.begin(), edges.end(), e,
        [](const Edge<K, T>& a, const Edge<K, T>& b) { return a.label < b.label; });
    
    edges.insert(it, e);
}

template<typename K, typename T>
std::shared_ptr<Node<K, T>> Node<K, T>::getLowerBoundEdge(typename K::value_type label, int* out_index) const {
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

template<typename K, typename T>
std::shared_ptr<LeafNode<K, T>> Node<K, T>::minimumLeaf(bool* found) const {
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

template<typename K, typename T>
std::shared_ptr<LeafNode<K, T>> Node<K, T>::maximumLeaf(bool* found) const {
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

template<typename K, typename T>
void Node<K, T>::updateMinMaxLeaves() {
    leaves_in_subtree = 0;
    minLeaf = nullptr;
    maxLeaf = nullptr;
    
    if (leaf != nullptr) {
        minLeaf = leaf;
    } else if (!edges.empty()) {
        minLeaf = edges[0].node->minLeaf;
    }
    
    if (!edges.empty()) {
        maxLeaf = edges.back().node->maxLeaf;
    }
    
    if (maxLeaf == nullptr && leaf != nullptr) {
        maxLeaf = leaf;
    }
}

template<typename K, typename T>
void Node<K, T>::computeLinks() {
    updateMinMaxLeaves();
    // Reset leaves_in_subtree and count properly like Go does
    leaves_in_subtree = 0;
    if (leaf != nullptr) {
        leaves_in_subtree++;
    }
    if (!edges.empty()) {
        // Link the current node's leaf to the first child's minLeaf if they're different
        if (minLeaf != nullptr && minLeaf != edges[0].node->minLeaf) {
            minLeaf->nextLeaf = edges[0].node->minLeaf;
            if (edges[0].node->minLeaf != nullptr) {
                edges[0].node->minLeaf->prevLeaf = minLeaf;
            }
        }
    }
    // Link consecutive child nodes and count leaves - use direct property access
    for (size_t i = 0; i < edges.size(); i++) {
        leaves_in_subtree += edges[i].node->leaves_in_subtree;
        auto maxLFirst = edges[i].node->maxLeaf;
        std::shared_ptr<LeafNode<K, T>> minLSecond = nullptr;
        if (i + 1 < edges.size()) {
            minLSecond = edges[i + 1].node->minLeaf;
        }
        if (maxLFirst != nullptr) {
            maxLFirst->nextLeaf = minLSecond;
        }
        if (minLSecond != nullptr) {
            minLSecond->prevLeaf = maxLFirst;
        }
    }
}

template<typename K, typename T>
bool Node<K, T>::isLeaf() const {
    return leaf != nullptr;
}

template<typename K, typename T>
bool Node<K, T>::Get(const K& search, T& result) const {
    if (search.empty()) {
        if (leaf) {
            result = leaf->val;
            return true;
        }
        return false;
    }

    for (const auto& edge : edges) {
        if (hasPrefix(search, edge.node->prefix)) {
            K newSearch(search.begin() + edge.node->prefix.size(), search.end());
            return edge.node->Get(newSearch, result);
        }
    }
    
    return false;
}

// Explicit template instantiations
template class Node<std::string, std::string>;
template class Node<std::string, int>;
template class Node<std::string, double>;
template class Node<std::vector<uint8_t>, std::string>;
template class LeafNode<std::string, std::string>;
template class LeafNode<std::string, int>;
template class LeafNode<std::string, double>;
template class LeafNode<std::vector<uint8_t>, std::string>;

template bool hasPrefix<std::string>(const std::string&, const std::string&);
template bool hasPrefix<std::vector<uint8_t>>(const std::vector<uint8_t>&, const std::vector<uint8_t>&);
template int longestPrefix<std::string>(const std::string&, const std::string&);
template int longestPrefix<std::vector<uint8_t>>(const std::vector<uint8_t>&, const std::vector<uint8_t>&);
template std::string concat<std::string>(const std::string&, const std::string&);
template std::vector<uint8_t> concat<std::vector<uint8_t>>(const std::vector<uint8_t>&, const std::vector<uint8_t>&); 