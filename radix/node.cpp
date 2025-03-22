//
// Created by Ashesh Vidyut on 22/03/25.
//

#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "iterator.hpp"
#include "rev_iterator.hpp"
#include "node.hpp"

// Forward declarations
typedef struct LeafNode LeafNode;
typedef struct Node Node;
typedef struct edge edge;

// Returns the key stored in the leaf node.
unsigned char *LeafNode_Key(LeafNode *n) {
    return n->key;
}

// Returns the value stored in the leaf node.
void *LeafNode_Value(LeafNode *n) {
    return n->val;
}

// Sets the next leaf pointer.
void LeafNode_SetNextLeaf(LeafNode *n, LeafNode *l) {
    n->nextLeaf = l;
}

// Returns the next leaf pointer.
LeafNode *LeafNode_GetNextLeaf(LeafNode *n) {
    return n->nextLeaf;
}

// Sets the previous leaf pointer.
void LeafNode_SetPrevLeaf(LeafNode *n, LeafNode *l) {
    n->prevLeaf = l;
}

// Returns the previous leaf pointer.
LeafNode *LeafNode_GetPrevLeaf(LeafNode *n) {
    return n->prevLeaf;
}

LeafNode* Node_MinimumLeaf(Node *n, bool *found) {
    if (n->minLeaf != NULL) {
        if (found)
            *found = true;
        return n->minLeaf;
    }
    if (found)
        *found = false;
    return NULL;
}

LeafNode* Node_MaximumLeaf(Node *n, bool *found) {
    if (n->maxLeaf != NULL) {
        if (found)
            *found = true;
        return n->maxLeaf;
    }
    if (found)
        *found = false;
    return NULL;
}

void Node_updateMinMaxLeaves(Node *n) {
    n->minLeaf = NULL;
    n->maxLeaf = NULL;
    if (n->leaf != NULL) {
        n->minLeaf = n->leaf;
    } else if (n->edges.len > 0) {
        n->minLeaf = n->edges.data[0].node->minLeaf;
    }
    if (n->edges.len > 0) {
        n->maxLeaf = n->edges.data[n->edges.len - 1].node->maxLeaf;
    }
    if (n->maxLeaf == NULL && n->leaf != NULL) {
        n->maxLeaf = n->leaf;
    }
}

void Node_computeLinks(Node *n) {
    Node_updateMinMaxLeaves(n);
    if (n->edges.len > 0) {
        if (n->minLeaf != n->edges.data[0].node->minLeaf) {
            LeafNode_SetNextLeaf(n->minLeaf, n->edges.data[0].node->minLeaf);
            if (n->edges.data[0].node->minLeaf != NULL) {
                LeafNode_SetPrevLeaf(n->edges.data[0].node->minLeaf, n->minLeaf);
            }
        }
    }
    for (size_t itr = 0; itr < n->edges.len; itr++) {
        bool foundMax = false;
        LeafNode *maxLFirst = Node_MaximumLeaf(n->edges.data[itr].node, &foundMax);

        bool foundMin = false;
        LeafNode *minLSecond = NULL;
        if (itr + 1 < n->edges.len) {
            minLSecond = Node_MinimumLeaf(n->edges.data[itr + 1].node, &foundMin);
        }
        if (maxLFirst != NULL) {
            LeafNode_SetNextLeaf(maxLFirst, minLSecond);
        }
        if (minLSecond != NULL) {
            LeafNode_SetPrevLeaf(minLSecond, maxLFirst);
        }
    }
}

void Node_addEdge(Node *n, edge e) {
    size_t num = n->edges.len;
    size_t low = 0, high = num;
    // Binary search to find insertion index such that:
    // n->edges.data[i].label >= e.label
    while (low < high) {
        size_t mid = (low + high) >> 1;
        if (n->edges.data[mid].label >= e.label)
            high = mid;
        else
            low = mid + 1;
    }
    size_t idx = low;

    // Ensure there is capacity to append a new edge.
    if (n->edges.len == n->edges.cap) {
        size_t new_cap = n->edges.cap ? n->edges.cap * 2 : 4;
        edge *new_data = (edge *)realloc(n->edges.data, new_cap * sizeof(edge));
        if (!new_data) {
            perror("realloc failed");
            exit(EXIT_FAILURE);
        }
        n->edges.data = new_data;
        n->edges.cap = new_cap;
    }

    // Append the new edge at the end.
    n->edges.data[n->edges.len] = e;
    n->edges.len++;

    // If the insertion index is not at the end, shift elements to insert e.
    if (idx != num) {
        memmove(&n->edges.data[idx+1], &n->edges.data[idx],
                (num - idx) * sizeof(edge));
        n->edges.data[idx] = e;
    }
}

void Node_replaceEdge(Node *n, edge e) {
    size_t num = n->edges.len;
    size_t low = 0, high = num;

    // Binary search for the insertion point.
    while (low < high) {
        size_t mid = (low + high) >> 1;
        if (n->edges.data[mid].label >= e.label)
            high = mid;
        else
            low = mid + 1;
    }
    size_t idx = low;

    if (idx < num && n->edges.data[idx].label == e.label) {
        n->edges.data[idx].node = e.node;
        return;
    }

    // If we reach here, no matching edge was found.
    fprintf(stderr, "replacing missing edge\n");
    exit(EXIT_FAILURE);
}

Node* Node_getEdge(Node *n, unsigned char label, int *out_index) {
    size_t num = n->edges.len;
    size_t low = 0, high = num;

    // Binary search for the edge.
    while (low < high) {
        size_t mid = (low + high) >> 1;
        if (n->edges.data[mid].label >= label)
            high = mid;
        else
            low = mid + 1;
    }
    size_t idx = low;

    if (idx < num && n->edges.data[idx].label == label) {
        if (out_index)
            *out_index = (int)idx;
        return n->edges.data[idx].node;
    }

    if (out_index)
        *out_index = -1;
    return NULL;
}


Node* Node_getLowerBoundEdge(Node *n, unsigned char label, int *out_index) {
    size_t num = n->edges.len;
    size_t low = 0, high = num;
    // Binary search for lower-bound
    while (low < high) {
        size_t mid = (low + high) >> 1;
        if (n->edges.data[mid].label >= label)
            high = mid;
        else
            low = mid + 1;
    }
    size_t idx = low;
    if (idx < num) {
        if (out_index)
            *out_index = (int)idx;
        return n->edges.data[idx].node;
    }
    if (out_index)
        *out_index = -1;
    return NULL;
}

void Node_delEdge(Node *n, unsigned char label) {
    size_t num = n->edges.len;
    size_t low = 0, high = num;
    // Binary search for the edge with the given label
    while (low < high) {
        size_t mid = (low + high) >> 1;
        if (n->edges.data[mid].label >= label)
            high = mid;
        else
            low = mid + 1;
    }
    size_t idx = low;
    if (idx < num && n->edges.data[idx].label == label) {
        // Shift elements left to remove the edge
        if (idx + 1 < num) {
            memmove(&n->edges.data[idx], &n->edges.data[idx + 1], (num - idx - 1) * sizeof(edge));
        }
        n->edges.len--;
    }
}

void *Node_Search(Node *n, unsigned char *k, size_t k_len, int *found) {
    unsigned char *search = k;
    size_t search_len = k_len;

    while (1) {
        // Check for key exhaustion.
        if (search_len == 0) {
            if (n && n->leaf) {
                if (found)
                    *found = 1;
                return n->leaf->val;
            }
            break;
        }

        // Look for an edge based on the first byte of the remaining key.
        int idx;
        Node *next = Node_getEdge(n, search[0], &idx);
        if (next == NULL) {
            break;
        }
        n = next;

        // Consume the node's prefix if it matches the start of 'search'.
        if (n->prefix_len <= search_len &&
            memcmp(search, n->prefix, n->prefix_len) == 0) {
            search += n->prefix_len;
            search_len -= n->prefix_len;
        } else {
            break;
        }
    }

    if (found)
        *found = 0;
    return NULL;
}

void *Node_Get(Node *n, unsigned char *k, size_t k_len, int *found) {
    return Node_Search(n, k, k_len, found);
}

// LongestPrefix searches for the longest prefix match.
// k is the key to search, and k_len its length.
LongestPrefixResult Node_LongestPrefix(Node *n, unsigned char *k, size_t k_len) {
    LeafNode *last = NULL;
    unsigned char *search = k;
    size_t search_len = k_len;

    for (;;) {
        // Look for a leaf node.
        if (n->leaf != NULL) {
            last = n->leaf;
        }

        // Check for key exhaustion.
        if (search_len == 0) {
            break;
        }

        int idx;
        // Look for an edge based on the first byte of the remaining key.
        Node *next = Node_getEdge(n, search[0], &idx);
        if (next == NULL) {
            break;
        }
        n = next;

        // Consume the search prefix if it matches.
        if (n->prefix_len <= search_len &&
            memcmp(search, n->prefix, n->prefix_len) == 0) {
            search += n->prefix_len;
            search_len -= n->prefix_len;
        } else {
            break;
        }
    }

    LongestPrefixResult res;
    if (last != NULL) {
        res.key = last->key;
        res.key_len = last->key_len;
        res.val = last->val;
        res.found = 1;
    } else {
        res.key = NULL;
        res.key_len = 0;
        res.val = NULL;
        res.found = 0;
    }
    return res;
}

// Returns a new iterator starting at node n.
Iterator *Node_Iterator(Node *n) {
    Iterator *it = (Iterator *)malloc(sizeof(Iterator));
    if (!it) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    it->node = n;
    it->leafNode = NULL;
    // Initialize other fields as needed.
    return it;
}

// Returns a new reverse iterator starting at node n.
ReverseIterator *Node_ReverseIterator(Node *n) {
    return ReverseIterator_New(n);
}

bool Node_isLeaf(Node *n) {
    return n->leaf != NULL;
}