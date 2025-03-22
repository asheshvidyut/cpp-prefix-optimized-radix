//
// Created by Ashesh Vidyut on 22/03/25.
//

#include <stdbool.h>
#include <regex.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include "iterator.hpp"

void Iterator_PatternMatch(Iterator *i, regex_t *regex) {
    i->patternMatch = true;
    i->pattern = regex;
}


void Iterator_SeekPrefixWatch(Iterator *i, unsigned char *prefix, size_t prefix_len) {
    // Wipe the stack.
    i->seekLowerBound = false;
    i->stack = NULL;       // or reset the dynamic array as needed
    i->stack_len = 0;
    i->stack_cap = 0;

    // Set the iterator's key to the prefix.
    i->key = prefix;
    i->key_len = prefix_len;

    // Start with the current node and the full prefix as the search.
    Node *n = i->node;
    unsigned char *search = prefix;
    size_t search_len = prefix_len;

    for (;;) {
        // Check for key exhaustion.
        if (search_len == 0) {
            i->node = n;
            return;
        }

        int idx;
        Node *child = Node_getEdge(n, search[0], &idx);
        if (child == NULL) {
            i->node = NULL;
            return;
        }
        n = child;

        // Consume the search prefix if the node's prefix matches the start of 'search'.
        if (n->prefix_len <= search_len &&
            memcmp(search, n->prefix, n->prefix_len) == 0) {
            search += n->prefix_len;
            search_len -= n->prefix_len;
        }
            // Otherwise, if the node's prefix starts with the remaining search bytes,
            // we have found the finest granularity.
        else if (n->prefix_len >= search_len &&
                 memcmp(n->prefix, search, search_len) == 0) {
            i->node = n;
            return;
        } else {
            i->node = NULL;
            return;
        }
    }
}

void Iterator_SeekPrefix(Iterator *i, unsigned char *prefix, size_t prefix_len) {
    Iterator_SeekPrefixWatch(i, prefix, prefix_len);
}

IteratorResult Iterator_Next(Iterator *i) {
    IteratorResult res = { .key = NULL, .key_len = 0, .val = NULL, .found = false };

    // If we haven't started iterating over leaves yet, get the minimum leaf.
    if (i->node != NULL && i->leafNode == NULL) {
        // Here we assume Node_MinimumLeaf simply returns i->node->leaf.
        // Adjust if you have a different function.
        bool found;
        i->leafNode = Node_MinimumLeaf(i->node, &found);
    }

    if (i->patternMatch) {
        // Pattern match branch: use POSIX regex to test each leaf's key.
        while (i->leafNode != NULL) {
            // regexec returns 0 on a match.
            int reti = regexec(i->pattern, (const char *)i->leafNode->key, 0, NULL, 0);
            if (reti == 0) {  // Match found.
                LeafNode *temp = i->leafNode;
                i->leafNode = LeafNode_GetNextLeaf(i->leafNode);
                if (i->leafNode == NULL) {
                    i->node = NULL;
                }
                res.key = temp->key;
                res.key_len = temp->key_len;
                res.val = temp->val;
                res.found = true;
                return res;
            } else {
                // Move to next leaf.
                i->leafNode = LeafNode_GetNextLeaf(i->leafNode);
                if (i->leafNode == NULL) {
                    i->node = NULL;
                }
            }
        }
    } else {
        // No pattern matching; check for the prefix match.
        while (i->leafNode != NULL) {
            // Check if the leaf's key starts with the iterator's key.
            if (i->key != NULL &&
                i->leafNode->key != NULL &&
                i->leafNode->key_len >= i->key_len &&
                memcmp(i->leafNode->key, i->key, i->key_len) == 0) {
                LeafNode *temp = i->leafNode;
                i->leafNode = LeafNode_GetNextLeaf(i->leafNode);
                if (i->leafNode == NULL) {
                    i->node = NULL;
                }
                res.key = temp->key;
                res.key_len = temp->key_len;
                res.val = temp->val;
                res.found = true;
                return res;
            } else {
                // If the current leaf doesn't match the prefix, iteration stops.
                i->leafNode = NULL;
                i->node = NULL;
                break;
            }
        }
    }

    // If we reach here, no matching leaf was found.
    i->leafNode = NULL;
    i->node = NULL;
    return res;
}
