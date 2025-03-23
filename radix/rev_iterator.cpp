//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "rev_iterator.hpp"
#include "node.hpp"      // Provides definitions for Node, LeafNode, etc.
#include <stdlib.h>
#include <string.h>
#include "iterator.hpp"
#include <stdio.h>

// NewReverseIterator returns a new ReverseIterator at a node.
ReverseIterator *ReverseIterator_New(Node *n) {
    ReverseIterator *ri = (ReverseIterator *)RedisModule_Alloc(sizeof(ReverseIterator));
    if (!ri) {
        fprintf(stderr, "malloc failed");
        exit(EXIT_FAILURE);
    }
    // Create a new underlying Iterator that starts at node n.
    ri->i = Node_Iterator(n);  // Assumes Iterator_New allocates and initializes an Iterator.
    return ri;
}

// SeekPrefixWatch updates the underlying iterator with the given prefix.
// In this C version, we assume Iterator_SeekPrefixWatch is defined to update the iterator state.
void ReverseIterator_SeekPrefixWatch(ReverseIterator *ri, unsigned char *prefix, size_t prefix_len) {
    Iterator_SeekPrefixWatch(ri->i, prefix, prefix_len);
}

// SeekPrefix simply delegates to SeekPrefixWatch.
void ReverseIterator_SeekPrefix(ReverseIterator *ri, unsigned char *prefix, size_t prefix_len) {
    ReverseIterator_SeekPrefixWatch(ri, prefix, prefix_len);
}

// Previous returns the previous node in reverse order.
// It uses the underlying iterator's leafNode and its GetPrevLeaf function.
IteratorResult ReverseIterator_Previous(ReverseIterator *ri) {
    // If the iterator's leafNode is not yet set and node is available, initialize it.
    if (ri->i->leafNode == NULL && ri->i->node != NULL) {
        // Use the node's maximum leaf as the starting point.
        bool found;
        ri->i->leafNode = Node_MaximumLeaf(ri->i->node, &found);
    }

    // Loop through the leaves.
    while (ri->i->leafNode != NULL) {
        // Check if the current leaf's key has the desired prefix.
        if (ri->i->key != NULL &&
            ri->i->leafNode->key != NULL &&
            ri->i->leafNode->key_len >= ri->i->key_len &&
            memcmp(ri->i->leafNode->key, ri->i->key, ri->i->key_len) == 0) {

            LeafNode *res = ri->i->leafNode;
            // Move to the previous leaf.
            ri->i->leafNode = LeafNode_GetPrevLeaf(ri->i->leafNode);
            if (ri->i->leafNode == NULL) {
                ri->i->node = NULL;
            }
            IteratorResult result;
            result.key = res->key;
            result.key_len = res->key_len;
            result.val = res->val;
            result.found = true;
            return result;
        } else {
            // No match: clear the iterator.
            ri->i->leafNode = NULL;
            ri->i->node = NULL;
            break;
        }
    }

    // No previous element found.
    IteratorResult result = { NULL, 0, NULL, false };
    return result;
}
