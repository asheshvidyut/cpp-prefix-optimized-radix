//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef NODE_H
#define NODE_H

#include <regex.h>  // if you use regex_t for patterns
#include "radix.hpp"
#include <stdbool.h>
#include "node.hpp"

// Definition of LeafNode.
typedef struct LeafNode {
    unsigned char *key;    // key data
    size_t key_len;        // key length
    void *val;             // generic pointer for the value
    struct LeafNode *nextLeaf;
    struct LeafNode *prevLeaf;
} LeafNode;

// Forward declaration of Node.
typedef struct Node Node;

// edge structure.
typedef struct edge {
    unsigned char label;
    Node *node;
} edge;

// Dynamic array of edge elements.
typedef struct {
    edge *data;    // pointer to an array of edges
    size_t len;    // current number of edges
    size_t cap;    // allocated capacity
} edges;

// Node structure.
typedef struct Node {
    LeafNode *leaf;
    LeafNode *minLeaf;
    LeafNode *maxLeaf;
    unsigned char *prefix;
    size_t prefix_len;
    edges edges;
} Node;

// Function prototypes.
Node *Node_getEdge(Node *n, unsigned char label, int *out_index);
void Node_replaceEdge(Node *n, edge e);
void Node_delEdge(Node *n, unsigned char label);
void *Node_Search(Node *n, unsigned char *k, size_t k_len, int *found);
void *Node_Get(Node *n, unsigned char *k, size_t k_len, int *found);

// Other Node-related function prototypes...

// Forward declarations for types.
typedef struct LeafNode LeafNode;
typedef struct Node Node;
typedef struct edge edge;

// Returns the key stored in the leaf node.
unsigned char *LeafNode_Key(LeafNode *n);

// Returns the value stored in the leaf node.
void *LeafNode_Value(LeafNode *n);

// Sets the next leaf pointer.
void LeafNode_SetNextLeaf(LeafNode *n, LeafNode *l);

// Returns the next leaf pointer.
LeafNode *LeafNode_GetNextLeaf(LeafNode *n);

// Sets the previous leaf pointer.
void LeafNode_SetPrevLeaf(LeafNode *n, LeafNode *l);

// Returns the previous leaf pointer.
LeafNode *LeafNode_GetPrevLeaf(LeafNode *n);

// --- Node function prototypes ---

// Returns the minimum leaf in the node; 'found' is set to true if one exists.
LeafNode* Node_MinimumLeaf(Node *n, bool *found);

// Returns the maximum leaf in the node; 'found' is set to true if one exists.
LeafNode* Node_MaximumLeaf(Node *n, bool *found);

// Updates the node's minLeaf and maxLeaf fields.
void Node_updateMinMaxLeaves(Node *n);

// Computes the doubly-linked list among leaf nodes.
void Node_computeLinks(Node *n);

// Adds an edge to the node in sorted order.
void Node_addEdge(Node *n, edge e);

// Returns the lower bound edge for the given label; index is output via out_index.
Node* Node_getLowerBoundEdge(Node *n, unsigned char label, int *out_index);

// --- Longest Prefix Match ---
//
// The result structure for LongestPrefix.
typedef struct {
    unsigned char *key;  // Pointer to the key.
    size_t key_len;      // Length of the key.
    void *val;           // Associated value.
    int found;           // 1 if a match was found, 0 otherwise.
} LongestPrefixResult;

// Searches for the longest prefix match in the node.
LongestPrefixResult Node_LongestPrefix(Node *n, unsigned char *k, size_t k_len);

// Forward declarations of helper functions that must be defined elsewhere.
bool Node_isLeaf(Node *n);  // Returns true if n->leaf is not NULL.

#endif // NODE_H
