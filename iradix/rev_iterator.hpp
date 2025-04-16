//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef REVERSE_ITERATOR_H
#define REVERSE_ITERATOR_H

#include <stddef.h>
#include <stdbool.h>
#include "iterator.hpp"  // This header should define Iterator and IteratorResult

typedef struct {
    Iterator *i;  // Underlying iterator
} ReverseIterator;

// Creates a new ReverseIterator starting at node n.
ReverseIterator *ReverseIterator_New(Node *n);

// Seeks the iterator to a given prefix (watch variant).
// In this C version, the function updates the iterator state.
void ReverseIterator_SeekPrefixWatch(ReverseIterator *ri, unsigned char *prefix, size_t prefix_len);

// Seeks the iterator to a given prefix.
void ReverseIterator_SeekPrefix(ReverseIterator *ri, unsigned char *prefix, size_t prefix_len);

// Returns the previous element in reverse order.
// The result is returned in an IteratorResult structure.
IteratorResult ReverseIterator_Previous(ReverseIterator *ri);

// Returns a new reverse iterator starting at node n.
ReverseIterator *Node_ReverseIterator(Node *n);

#endif // REVERSE_ITERATOR_H
