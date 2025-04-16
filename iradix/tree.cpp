//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "tree.hpp"
#include <algorithm>
#include <cstring>

// This file is intentionally left mostly empty as the implementation is now in the header file.
// The template-based implementation is defined in tree.hpp.

// If you need to add any non-template functions or implementations that can't be in the header,
// you can add them here.

// Example of a non-template helper function that might be needed:
void initializeTree() {
    // Initialize any global state or resources needed for radix trees
    // This could include:
    // - Setting up memory pools
    // - Initializing thread-local storage
    // - Setting up logging or debugging infrastructure
    // - Any other global initialization needed for the radix tree implementation
}

// Constructor implementation
template<typename K, typename T>
Tree<K, T>::Tree() : root(std::make_shared<Node<K, T>>()), size(0) {
    root->mutateCh = std::make_shared<std::condition_variable>();
}

template<typename K, typename T>
std::shared_ptr<Node<K, T>> Tree<K, T>::getRoot() const {
    return root;
}

template<typename K, typename T>
int Tree<K, T>::len() const {
    return size;
}

// Transaction constructor
template<typename K, typename T>
Tree<K, T>::Transaction::Transaction(Tree<K, T>& t) : root(t.root), size(t.size), tree(t) {}

// Transaction methods
template<typename K, typename T>
std::tuple<std::shared_ptr<Node<K, T>>, std::optional<T>, bool> Tree<K, T>::Transaction::insert(
    std::shared_ptr<Node<K, T>> n,
    const K& k,
    const K& search,
    const T& v) {
    
    std::optional<T> oldVal;
    bool didUpdate = false;

    // Handle key exhaustion
    if (search.empty()) {
        if (n->leaf) {
            // Update existing leaf
            oldVal = n->leaf->val;
            didUpdate = true;
        }

        // Create a new leaf
        n->leaf = std::make_shared<LeafNode<K, T>>(k, v);
        if (!didUpdate) {
            size++;
        }
        return {n, oldVal, didUpdate};
    }

    // Look for an edge
    int idx;
    auto child = n->getEdge(search[0], &idx);

    // No edge, create one
    if (!child) {
        auto newNode = std::make_shared<Node<K, T>>();
        newNode->mutateCh = std::make_shared<std::condition_variable>();
        newNode->leaf = std::make_shared<LeafNode<K, T>>(k, v);
        newNode->prefix = search;

        Edge<K, T> e;
        e.label = search[0];
        e.node = newNode;
        n->addEdge(e);

        size++;
        return {n, std::nullopt, false};
    }

    // Determine longest prefix of the search key on match
    int commonPrefix = longestPrefix(search, child->prefix);
    if (commonPrefix == child->prefix.size()) {
        // Consume the search prefix
        K newSearch(search.begin() + commonPrefix, search.end());
        auto [newChild, oldVal, didUpdate] = insert(child, k, newSearch, v);
        if (newChild != child) {
            Edge<K, T> e = n->edges[idx];
            e.node = newChild;
            n->replaceEdge(e);
        }
        return {n, oldVal, didUpdate};
    }

    // Split the node
    auto splitNode = std::make_shared<Node<K, T>>();
    splitNode->mutateCh = std::make_shared<std::condition_variable>();
    splitNode->prefix = K(search.begin(), search.begin() + commonPrefix);

    // Restore the existing child node
    auto modChild = child;
    modChild->prefix = K(child->prefix.begin() + commonPrefix, child->prefix.end());
    Edge<K, T> e1;
    e1.label = modChild->prefix[0];
    e1.node = modChild;
    splitNode->addEdge(e1);

    // Create a new leaf node
    auto leaf = std::make_shared<LeafNode<K, T>>(k, v);

    // If the new key is a subset, add to this node
    K remainingSearch(search.begin() + commonPrefix, search.end());
    if (remainingSearch.empty()) {
        splitNode->leaf = leaf;
    } else {
        // Create a new edge for the node
        auto newNode = std::make_shared<Node<K, T>>();
        newNode->mutateCh = std::make_shared<std::condition_variable>();
        newNode->leaf = leaf;
        newNode->prefix = remainingSearch;

        Edge<K, T> e2;
        e2.label = remainingSearch[0];
        e2.node = newNode;
        splitNode->addEdge(e2);
    }

    // Replace the original edge
    Edge<K, T> e;
    e.label = search[0];
    e.node = splitNode;
    n->replaceEdge(e);

    size++;
    return {n, std::nullopt, false};
}

template<typename K, typename T>
typename Tree<K, T>::Transaction::DeleteResult Tree<K, T>::Transaction::del(
    std::shared_ptr<Node<K, T>> parent,
    std::shared_ptr<Node<K, T>> n,
    const K& search) {
    
    DeleteResult result;
    result.node = nullptr;
    result.leaf = nullptr;

    // If we're at the end of the search, we're deleting
    if (search.empty()) {
        if (n->leaf) {
            // Delete the leaf
            result.leaf = n->leaf;
            n->leaf = nullptr;
            n->minLeaf = nullptr;
            n->maxLeaf = nullptr;
            size--;

            // If the node has no edges, it can be removed
            if (n->edges.empty()) {
                return result;
            }

            // If the node has only one edge, merge with the child
            if (n->edges.size() == 1) {
                auto child = n->edges[0].node;
                child->prefix = concat(n->prefix, child->prefix);
                result.node = child;
                return result;
            }

            // Otherwise, just update the node
            n->updateMinMaxLeaves();
            result.node = n;
            return result;
        }
        return result;
    }

    // Look for an edge
    if (search.empty()) {
        result.node = n;
        return result;
    }
    int idx;
    auto child = n->getEdge(search[0], &idx);
    if (child) {
        // Consume the search prefix
        K newSearch = search;
        if (!child->prefix.empty() && !newSearch.empty()) {
            newSearch.erase(newSearch.begin(), newSearch.begin() + std::min(child->prefix.size(), newSearch.size()));
        }

        // Recurse
        auto childResult = del(n, child, newSearch);
        
        if (childResult.node != child) {
            // Child was modified, update the edge
            if (childResult.node) {
                Edge<K, T> e = n->edges[idx];
                e.node = childResult.node;
                n->replaceEdge(e);
            } else {
                n->delEdge(search[0]);
            }
        }

        // If the node has no edges and no leaf, it can be removed
        if (n->edges.empty() && !n->leaf) {
            result.node = nullptr;
        } else {
            // Otherwise, just update the node
            n->updateMinMaxLeaves();
            result.node = n;
        }

        result.leaf = childResult.leaf;
        return result;
    }

    // No edge found, nothing to delete
    result.node = n;
    return result;
}

template<typename K, typename T>
typename Tree<K, T>::Transaction::DeletePrefixResult Tree<K, T>::Transaction::deletePrefix(
    std::shared_ptr<Node<K, T>> n,
    const K& search) {
    
    DeletePrefixResult result;
    result.node = nullptr;
    result.numDeletions = 0;

    // If we're at the end of the search, delete the entire subtree
    if (search.empty()) {
        // Count the leaves in the subtree
        int count = 0;
        std::function<void(std::shared_ptr<Node<K, T>>)> countLeaves = 
            [&count, &countLeaves](std::shared_ptr<Node<K, T>> node) {
                if (node->leaf) count++;
                for (const auto& edge : node->edges) {
                    countLeaves(edge.node);
                }
            };
        
        countLeaves(n);
        result.numDeletions = count;
        size -= count;
        return result;
    }

    // Look for an edge
    if (search.empty()) {
        result.node = n;
        return result;
    }
    int idx;
    auto child = n->getEdge(search[0], &idx);
    if (child) {
        // Check if the child's prefix is a prefix of the search
        if (hasPrefix(search, child->prefix)) {
            // Consume the search prefix
            K newSearch = search;
            if (!child->prefix.empty() && !newSearch.empty()) {
                newSearch.erase(newSearch.begin(), newSearch.begin() + std::min(child->prefix.size(), newSearch.size()));
            }

            // Recurse
            auto childResult = deletePrefix(child, newSearch);
            
            if (childResult.node != child) {
                // Child was modified, update the edge
                if (childResult.node) {
                    Edge<K, T> e = n->edges[idx];
                    e.node = childResult.node;
                    n->replaceEdge(e);
                } else {
                    n->delEdge(search[0]);
                }
            }

            // If the node has no edges and no leaf, it can be removed
            if (n->edges.empty() && !n->leaf) {
                result.node = nullptr;
            } else {
                // Otherwise, just update the node
                n->updateMinMaxLeaves();
                result.node = n;
            }

            result.numDeletions = childResult.numDeletions;
            return result;
        } else if (hasPrefix(child->prefix, search)) {
            // The search is a prefix of the child's prefix, delete the entire child
            int count = 0;
            std::function<void(std::shared_ptr<Node<K, T>>)> countLeaves = 
                [&count, &countLeaves](std::shared_ptr<Node<K, T>> node) {
                    if (node->leaf) count++;
                    for (const auto& edge : node->edges) {
                        countLeaves(edge.node);
                    }
                };
            
            countLeaves(child);
            result.numDeletions = count;
            size -= count;
            n->delEdge(search[0]);
            result.node = n;
            return result;
        }
    }

    // No matching prefix found
    result.node = n;
    return result;
}

template<typename K, typename T>
void Tree<K, T>::Transaction::mergeChild(std::shared_ptr<Node<K, T>> n) {
    if (n->edges.size() != 1) return;

    auto child = n->edges[0].node;
    child->prefix = concat(n->prefix, child->prefix);
    n->edges = child->edges;
    n->leaf = child->leaf;
    n->minLeaf = child->minLeaf;
    n->maxLeaf = child->maxLeaf;
}

template<typename K, typename T>
int Tree<K, T>::Transaction::trackChannelsAndCount(std::shared_ptr<Node<K, T>> n) {
    int count = 0;
    if (n->leaf) count++;
    for (const auto& edge : n->edges) {
        count += trackChannelsAndCount(edge.node);
    }
    return count;
}

template<typename K, typename T>
typename Tree<K, T>::Transaction Tree<K, T>::Transaction::clone() {
    return *this;
}

template<typename K, typename T>
Tree<K, T> Tree<K, T>::Transaction::commit() {
    tree.root = root;
    tree.size = size;
    return tree;
}

template<typename K, typename T>
Tree<K, T> Tree<K, T>::Transaction::commitOnly() {
    tree.root = root;
    tree.size = size;
    return tree;
}

// Tree methods
template<typename K, typename T>
typename Tree<K, T>::Transaction Tree<K, T>::txn() {
    return Transaction(*this);
}

template<typename K, typename T>
std::tuple<Tree<K, T>, std::optional<T>, bool> Tree<K, T>::insert(const K& k, const T& v) {
    auto t = txn();
    auto [newRoot, oldVal, didUpdate] = t.insert(root, k, k, v);
    t.root = newRoot;
    return {t.commit(), oldVal, didUpdate};
}

template<typename K, typename T>
std::tuple<Tree<K, T>, std::optional<T>, bool> Tree<K, T>::del(const K& k) {
    auto t = txn();
    auto result = t.del(nullptr, root, k);
    t.root = result.node ? result.node : root;
    return {t.commit(), result.leaf ? std::optional<T>(result.leaf->val) : std::nullopt, result.leaf != nullptr};
}

template<typename K, typename T>
std::tuple<Tree<K, T>, bool, int> Tree<K, T>::deletePrefix(const K& k) {
    auto t = txn();
    auto result = t.deletePrefix(root, k);
    t.root = result.node ? result.node : root;
    return {t.commit(), result.numDeletions > 0, result.numDeletions};
}

template<typename K, typename T>
std::optional<T> Tree<K, T>::Get(const K& search) const {
    return root->Get(search);
}

template<typename K, typename T>
Iterator<K, T> Tree<K, T>::iterator() const {
    return Iterator<K, T>(root);
}

// Explicit template instantiations
template class Tree<std::string, std::string>;
template class Tree<std::string, int>;
template class Tree<std::string, double>;
template class Tree<std::vector<uint8_t>, std::string>; 