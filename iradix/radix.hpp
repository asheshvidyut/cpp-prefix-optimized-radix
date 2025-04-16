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

    friend class Transaction;

public:
    RadixTree();
    std::shared_ptr<Node<K, T>> getRoot() const;
    int len() const;

    // Transaction class for atomic operations
    class Transaction {
    protected:
        std::shared_ptr<Node<K, T>> root;
        int size;
        RadixTree<K, T>& tree;

        friend class RadixTree;

    public:
        Transaction(RadixTree<K, T>& t);

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
        RadixTree<K, T> commit();
        RadixTree<K, T> commitOnly();
    };

    Transaction txn();
    std::tuple<RadixTree<K, T>, std::optional<T>, bool> insert(const K& k, const T& v);
    std::tuple<RadixTree<K, T>, std::optional<T>, bool> del(const K& k);
    std::tuple<RadixTree<K, T>, bool, int> deletePrefix(const K& k);
    std::optional<T> Get(const K& search) const;
    Iterator<K, T> iterator() const;
};

// Non-template function declaration
void initializeRadixTree();

// Explicit template instantiation declarations
extern template class RadixTree<std::string, std::string>;
extern template class RadixTree<std::string, int>;
extern template class RadixTree<std::string, double>;

#endif // RADIX_H
