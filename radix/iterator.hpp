//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef ITERATOR_H
#define ITERATOR_H

#include "node.hpp"
#include <stdbool.h>
#include <stddef.h>
#include <regex.h>
#include "node.hpp"      // Contains declarations for Node, LeafNode, etc.
#include <stdbool.h>
#include <regex.h>
#include <stddef.h>

// Iterator structure.
typedef struct {
    Node *node;
    edges *stack;       // Dynamic array (stack) of edges.
    size_t stack_len;
    size_t stack_cap;
    LeafNode *leafNode;
    unsigned char *key;
    size_t key_len;
    bool seekLowerBound;
    bool patternMatch;
    regex_t *pattern;   // Pointer to compiled regular expression.
} Iterator;


typedef struct {
    unsigned char *key;  // Pointer to the key bytes.
    size_t key_len;      // Length of the key.
    void *val;           // Associated value.
    bool found;          // True if a valid result is returned.
} IteratorResult;

// Function prototypes for iterator functionality.
// e.g., Iterator *Iterator_new(Node *n); void Iterator_next(Iterator *it); etc.

// Sets pattern matching for the iterator.
void Iterator_PatternMatch(Iterator *i, regex_t *regex);

// Seeks the iterator to a given prefix (watch variant).
void Iterator_SeekPrefixWatch(Iterator *i, unsigned char *prefix, size_t prefix_len);

// Seeks the iterator to a given prefix.
void Iterator_SeekPrefix(Iterator *i, unsigned char *prefix, size_t prefix_len);

// Returns the next element from the iterator.
IteratorResult Iterator_Next(Iterator *i);

// Returns a new iterator starting at node n.
Iterator *Node_Iterator(Node *n);

#endif // ITERATOR_H