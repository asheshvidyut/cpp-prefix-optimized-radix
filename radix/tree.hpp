//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef TREE_H
#define TREE_H

#include "node.hpp"
#include "iterator.cpp"
#include <memory>
#include <vector>
#include <functional>
#include <optional>
#include <tuple>

template<typename K>
K concat(const K& a, const K& b);  // Implementation in node.hpp

template<typename K, typename T>
class Node;

template<typename K, typename T>
class LeafNode;

// Result structure for delete operations
template<typename K, typename T>
struct DeleteResult {
    std::shared_ptr<Node<K, T>> node;
    std::shared_ptr<LeafNode<K, T>> leaf;
};

// Result structure for delete prefix operations
template<typename K, typename T>
struct DeletePrefixResult {
    std::shared_ptr<Node<K, T>> node;
    int numDeletions;
};

// Tree class
template<typename K, typename T>
class Tree {
private:
    std::shared_ptr<Node<K, T>> root;
    int size;

    friend class Transaction;

public:
    Tree() : root(std::make_shared<Node<K, T>>()), size(0) {
        root->mutateCh = std::make_shared<std::condition_variable>();
    }

    std::shared_ptr<Node<K, T>> getRoot() const {
        return root;
    }

    int len() const {
        return size;
    }

    // Transaction class for atomic operations
    class Transaction {
    protected:
        std::shared_ptr<Node<K, T>> root;
        int size;
        Tree<K, T>& tree;

        friend class Tree;

    public:
        Transaction(Tree<K, T>& t) : root(t.root), size(t.size), tree(t) {}

        std::tuple<std::shared_ptr<Node<K, T>>, std::optional<T>, bool> insert(
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

        DeleteResult<K, T> del(std::shared_ptr<Node<K, T>> parent, std::shared_ptr<Node<K, T>> n, const K& search) {
            DeleteResult<K, T> result;
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
                    if (child->prefix.size() < newSearch.size()) {
                        K searchPrefix(newSearch.begin(), newSearch.begin() + child->prefix.size());
                        if (child->prefix != searchPrefix) {
                            result.node = n;
                            return result;
                        }
                        newSearch = K(newSearch.begin() + child->prefix.size(), newSearch.end());
                    } else {
                        if (child->prefix != newSearch) {
                            result.node = n;
                            return result;
                        }
                        newSearch.clear();
                    }
                }

                // Recursively delete
                auto delResult = del(n, child, newSearch);
                if (delResult.node) {
                    Edge<K, T> e = n->edges[idx];
                    e.node = delResult.node;
                    n->replaceEdge(e);
                } else {
                    n->edges.erase(n->edges.begin() + idx);
                    if (n->edges.empty() && !n->leaf) {
                        return result;
                    }
                }

                // Update min/max leaves
                n->updateMinMaxLeaves();
                result.node = n;
                result.leaf = delResult.leaf;
                return result;
            }

            result.node = n;
            return result;
        }

        DeletePrefixResult<K, T> deletePrefix(std::shared_ptr<Node<K, T>> n, const K& search) {
            DeletePrefixResult<K, T> result;
            result.node = nullptr;
            result.numDeletions = 0;

            // Handle key exhaustion
            if (search.empty()) {
                // Delete all leaves under this node
                int count = 0;
                std::function<void(std::shared_ptr<Node<K, T>>)> countLeaves = [&count, &countLeaves](std::shared_ptr<Node<K, T>> node) {
                    if (node->leaf) {
                        count++;
                    }
                    for (const auto& edge : node->edges) {
                        countLeaves(edge.node);
                    }
                };
                countLeaves(n);
                size -= count;
                result.numDeletions = count;
                return result;
            }

            // Look for an edge
            int idx;
            auto child = n->getEdge(search[0], &idx);
            if (child) {
                // Consume the search prefix
                K newSearch = search;
                if (!child->prefix.empty() && !newSearch.empty()) {
                    if (child->prefix.size() < newSearch.size()) {
                        K searchPrefix(newSearch.begin(), newSearch.begin() + child->prefix.size());
                        if (child->prefix != searchPrefix) {
                            result.node = n;
                            return result;
                        }
                        newSearch = K(newSearch.begin() + child->prefix.size(), newSearch.end());
                    } else {
                        if (child->prefix != newSearch) {
                            result.node = n;
                            return result;
                        }
                        newSearch.clear();
                    }
                }

                // Recursively delete
                auto delResult = deletePrefix(child, newSearch);
                if (delResult.node) {
                    Edge<K, T> e = n->edges[idx];
                    e.node = delResult.node;
                    n->replaceEdge(e);
                } else {
                    n->edges.erase(n->edges.begin() + idx);
                    if (n->edges.empty() && !n->leaf) {
                        return result;
                    }
                }

                // Update min/max leaves
                n->updateMinMaxLeaves();
                result.node = n;
                result.numDeletions = delResult.numDeletions;
                return result;
            }

            result.node = n;
            return result;
        }

        void mergeChild(std::shared_ptr<Node<K, T>> n) {
            if (n->edges.size() != 1) {
                return;
            }

            auto child = n->edges[0].node;
            child->prefix = concat(n->prefix, child->prefix);
            n = child;
        }

        int trackChannelsAndCount(std::shared_ptr<Node<K, T>> n) {
            int count = 0;
            std::function<void(std::shared_ptr<Node<K, T>>)> countLeaves = [&count, &countLeaves](std::shared_ptr<Node<K, T>> node) {
                if (node->leaf) {
                    count++;
                }
                for (const auto& edge : node->edges) {
                    countLeaves(edge.node);
                }
            };
            countLeaves(n);
            return count;
        }

        Transaction clone() {
            return Transaction(tree);
        }

        Tree<K, T> commit() {
            tree.root = root;
            tree.size = size;
            return tree;
        }

        Tree<K, T> commitOnly() {
            Tree<K, T> newTree;
            newTree.root = root;
            newTree.size = size;
            return newTree;
        }
    };

    Transaction txn() {
        return Transaction(*this);
    }

    std::tuple<Tree<K, T>, std::optional<T>, bool> insert(const K& k, const T& v) {
        auto txn = this->txn();
        auto [newRoot, oldVal, didUpdate] = txn.insert(root, k, k, v);
        auto newTree = txn.commitOnly();
        return {newTree, oldVal, didUpdate};
    }

    std::tuple<Tree<K, T>, std::optional<T>, bool> del(const K& k) {
        auto txn = this->txn();
        auto result = txn.del(nullptr, root, k);
        auto newTree = txn.commitOnly();
        return {newTree, result.leaf ? std::optional<T>(result.leaf->val) : std::nullopt, result.leaf != nullptr};
    }

    std::tuple<Tree<K, T>, bool, int> deletePrefix(const K& k) {
        auto txn = this->txn();
        auto result = txn.deletePrefix(root, k);
        auto newTree = txn.commitOnly();
        return {newTree, result.node != nullptr, result.numDeletions};
    }

    std::optional<T> Get(const K& search) const {
        auto n = root;
        K remainingSearch = search;

        while (n) {
            // Handle leaf nodes
            if (n->leaf && n->leaf->key == search) {
                return n->leaf->val;
            }

            // Consume the prefix
            if (!n->prefix.empty() && !remainingSearch.empty()) {
                if (n->prefix.size() < remainingSearch.size()) {
                    K searchPrefix(remainingSearch.begin(), remainingSearch.begin() + n->prefix.size());
                    if (n->prefix != searchPrefix) {
                        return std::nullopt;
                    }
                    remainingSearch = K(remainingSearch.begin() + n->prefix.size(), remainingSearch.end());
                } else {
                    if (n->prefix != remainingSearch) {
                        return std::nullopt;
                    }
                    remainingSearch.clear();
                }
            }

            // Handle key exhaustion
            if (remainingSearch.empty()) {
                return n->leaf ? std::optional<T>(n->leaf->val) : std::nullopt;
            }

            // Look for an edge
            int idx;
            auto child = n->getEdge(remainingSearch[0], &idx);
            if (!child) {
                return std::nullopt;
            }
            n = child;
        }

        return std::nullopt;
    }

    Iterator<K, T> iterator() const {
        return Iterator<K, T>(root);
    }
};

// Non-template function declaration
void initializeTree();

#endif // TREE_H
