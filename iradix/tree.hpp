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

template<typename K>
K concat(const K& a, const K& b);  // Implementation in node.hpp

template<typename K, typename T>
class Node;

template<typename K, typename T>
class LeafNode;

// Tree class
template<typename K, typename T>
class Tree {
private:
    std::shared_ptr<Node<K, T>> root;
    int size;

    friend class Transaction;

public:
    Tree();
    std::shared_ptr<Node<K, T>> getRoot() const;
    int len() const;

    // Transaction class for atomic operations
    class Transaction {
    protected:
        std::shared_ptr<Node<K, T>> root;
        int size;
        Tree<K, T>& tree;

        friend class Tree;

    public:
        Transaction(Tree<K, T>& t);

        std::tuple<std::shared_ptr<Node<K, T>>, std::optional<T>, bool> insert(
            std::shared_ptr<Node<K, T>> n,
            const K& k,
            const K& search,
            const T& v);

        struct DeleteResult {
            std::shared_ptr<Node<K, T>> node;
            std::shared_ptr<LeafNode<K, T>> leaf;
        };

        DeleteResult del(std::shared_ptr<Node<K, T>> parent, std::shared_ptr<Node<K, T>> n, const K& search);

        struct DeletePrefixResult {
            std::shared_ptr<Node<K, T>> node;
            int numDeletions;
        };

        DeletePrefixResult deletePrefix(std::shared_ptr<Node<K, T>> n, const K& search);
        void mergeChild(std::shared_ptr<Node<K, T>> n);
        int trackChannelsAndCount(std::shared_ptr<Node<K, T>> n);
        Transaction clone();
        Tree<K, T> commit();
        Tree<K, T> commitOnly();
    };

    Transaction txn();
    std::tuple<Tree<K, T>, std::optional<T>, bool> insert(const K& k, const T& v);
    std::tuple<Tree<K, T>, std::optional<T>, bool> del(const K& k);
    std::tuple<Tree<K, T>, bool, int> deletePrefix(const K& k);
    std::optional<T> Get(const K& search) const;
    Iterator<K, T> iterator() const;
};

// Non-template function declaration
void initializeTree();

// Explicit template instantiation declarations
extern template class Tree<std::string, std::string>;
extern template class Tree<std::string, int>;
extern template class Tree<std::string, double>;

#endif // TREE_H
