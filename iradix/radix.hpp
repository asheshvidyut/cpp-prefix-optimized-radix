//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef RADIX_H
#define RADIX_H

#include "node.hpp"
#include "iterator.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <optional>

// Forward declarations
template<typename K>
bool hasPrefix(const K& str, const K& prefix);

template<typename K>
K concat(const K& a, const K& b);  // Implementation in node.hpp

template<typename K, typename T>
class Node;

template<typename K, typename T>
class LeafNode;

// Tree class
template<typename K, typename T>
class RadixTree {
private:
    std::shared_ptr<Node<K, T>> root;
    int size;

    friend class Transaction;  // Add friend declaration

public:
    RadixTree() : root(std::make_shared<Node<K, T>>()), size(0) {
        root->mutateCh = std::make_shared<std::condition_variable>();
    }

    // Returns the root node
    std::shared_ptr<Node<K, T>> getRoot() const {
        return root;
    }

    // Returns the number of elements in the tree
    int len() const {
        return size;
    }

    // Transaction class for atomic operations
    class Transaction {
    protected:
        std::shared_ptr<Node<K, T>> root;
        int size;
        RadixTree<K, T>& tree;

        friend class RadixTree;  // Add RadixTree as friend

    public:
        Transaction(RadixTree<K, T>& t) : root(t.root), size(t.size), tree(t) {}

        // Inserts a key/value pair into the transaction
        std::tuple<std::shared_ptr<Node<K, T>>, std::optional<T>, bool> insert(
            std::shared_ptr<Node<K, T>> n,
            const K& k,
            const K& search,
            const T& v) {
            
            std::optional<T> oldVal;
            bool didUpdate = false;

            // If we're at the end of the search, we're inserting
            if (search.empty()) {
                if (n->leaf) {
                    // Update existing leaf
                    oldVal = n->leaf->val;
                    n->leaf->val = v;
                    didUpdate = true;
                } else {
                    // Create a new leaf
                    n->leaf = std::make_shared<LeafNode<K, T>>(k, v);
                    n->minLeaf = n->leaf;
                    n->maxLeaf = n->leaf;
                    size++;
                }
                return {n, oldVal, didUpdate};
            }

            // Look for an edge
            if (search.empty()) {
                return {n, std::nullopt, false};
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
                auto [newChild, childOldVal, childDidUpdate] = insert(child, k, newSearch, v);
                
                if (newChild != child) {
                    // Child was modified, update the edge
                    Edge<K, T> e = n->edges[idx];
                    e.node = newChild;
                    n->replaceEdge(e);
                }

                if (childDidUpdate) {
                    didUpdate = true;
                    oldVal = childOldVal;
                }

                return {n, oldVal, didUpdate};
            }

            // No edge found, create a new node
            auto newNode = std::make_shared<Node<K, T>>();
            newNode->prefix = search;
            newNode->leaf = std::make_shared<LeafNode<K, T>>(k, v);
            newNode->minLeaf = newNode->leaf;
            newNode->maxLeaf = newNode->leaf;
            newNode->mutateCh = std::make_shared<std::condition_variable>();

            // Add the edge
            if (!search.empty()) {
                Edge<K, T> e;
                e.label = search[0];
                e.node = newNode;
                n->addEdge(e);
            }

            size++;
            return {n, std::nullopt, false};
        }

        // Deletes a key from the tree recursively
        struct DeleteResult {
            std::shared_ptr<Node<K, T>> node;
            std::shared_ptr<LeafNode<K, T>> leaf;
        };

        DeleteResult del(std::shared_ptr<Node<K, T>> parent, std::shared_ptr<Node<K, T>> n, const K& search) {
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

        // Deletes all nodes under a given prefix
        struct DeletePrefixResult {
            std::shared_ptr<Node<K, T>> node;
            int numDeletions;
        };

        DeletePrefixResult deletePrefix(std::shared_ptr<Node<K, T>> n, const K& search) {
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

        // Merges the given node with its single child
        void mergeChild(std::shared_ptr<Node<K, T>> n) {
            if (n->edges.size() != 1) return;

            auto child = n->edges[0].node;
            child->prefix = concat(n->prefix, child->prefix);
            n->edges = child->edges;
            n->leaf = child->leaf;
            n->minLeaf = child->minLeaf;
            n->maxLeaf = child->maxLeaf;
        }

        // Traverses the subtree at n, tracking channels and counting leaves
        int trackChannelsAndCount(std::shared_ptr<Node<K, T>> n) {
            int count = 0;
            if (n->leaf) count++;
            for (const auto& edge : n->edges) {
                count += trackChannelsAndCount(edge.node);
            }
            return count;
        }

        // Clones the transaction
        Transaction clone() {
            return *this;
        }

        // Commits the transaction and returns a new Tree
        RadixTree<K, T> commit() {
            tree.root = root;
            tree.size = size;
            return tree;
        }

        // Commits the transaction without issuing notifications
        RadixTree<K, T> commitOnly() {
            tree.root = root;
            tree.size = size;
            return tree;
        }
    };

    // Starts a new transaction on the tree
    Transaction txn() {
        return Transaction(*this);
    }

    // Inserts a key/value pair into the tree (transactionally)
    std::tuple<RadixTree<K, T>, std::optional<T>, bool> insert(const K& k, const T& v) {
        auto t = txn();
        auto [newRoot, oldVal, didUpdate] = t.insert(root, k, k, v);
        t.root = newRoot;
        return {t.commit(), oldVal, didUpdate};
    }

    // Deletes a key from the tree
    std::tuple<RadixTree<K, T>, std::optional<T>, bool> del(const K& k) {
        auto t = txn();
        auto result = t.del(nullptr, root, k);
        t.root = result.node ? result.node : root;
        return {t.commit(), result.leaf ? std::optional<T>(result.leaf->val) : std::nullopt, result.leaf != nullptr};
    }

    // Deletes all nodes starting with the given prefix
    std::tuple<RadixTree<K, T>, bool, int> deletePrefix(const K& k) {
        auto t = txn();
        auto result = t.deletePrefix(root, k);
        t.root = result.node ? result.node : root;
        return {t.commit(), result.numDeletions > 0, result.numDeletions};
    }

    // Looks up a key in the tree
    std::optional<T> get(const K& k) const {
        auto n = root;
        K search = k;

        while (true) {
            // If we're at the end of the search, we're done
            if (search.empty()) {
                if (n->leaf) {
                    return std::optional<T>(n->leaf->val);
                }
                return std::nullopt;
            }

            // Look for an edge
            if (search.empty()) {
                return std::nullopt;
            }
            auto child = n->getEdge(search[0]);
            if (!child) {
                return std::nullopt;
            }

            // Consume the search prefix
            if (hasPrefix(search, child->prefix)) {
                search.erase(search.begin(), search.begin() + child->prefix.size());
                n = child;
            } else if (hasPrefix(child->prefix, search)) {
                if (child->leaf) {
                    return std::optional<T>(child->leaf->val);
                }
                return std::nullopt;
            } else {
                return std::nullopt;
            }
        }
    }

    // Creates an iterator for the tree
    Iterator<K, T> iterator() const {
        return Iterator<K, T>(root);
    }
};

// Helper function to check if one sequence is a prefix of another
template<typename K>
bool hasPrefix(const K& str, const K& prefix) {
    if (prefix.size() > str.size()) return false;
    return std::equal(prefix.begin(), prefix.end(), str.begin());
}

#endif //RADIX_H
