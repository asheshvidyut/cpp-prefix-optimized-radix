//
// Created by Ashesh Vidyut on 22/03/25.
//
#include <stdlib.h>
#include <stdio.h>
#include "node.hpp"
#include "radix.hpp"
#include "string.h"
#include "redismodule.h"

// Forward declaration of Node; its definition should be provided elsewhere.
typedef struct Node Node;

// New returns an empty Tree.
Tree *Tree_New(void) {
    Tree *t = (Tree *)RedisModule_Alloc(sizeof(Tree));
    if (!t) {
        perror("malloc failed for Tree");
        return NULL;
    }
    t->root = (Node *)RedisModule_Alloc(sizeof(Node));
    if (!t->root) {
        perror("malloc failed for Node");
        free(t);
        return NULL;
    }
    // Initialize Node fields as needed.
    t->size = 0;
    return t;
}

int Tree_Len(Tree *t) {
    return t->size;
}

Txn *Tree_Txn(Tree *t) {
    Txn *txn = (Txn *)RedisModule_Alloc(sizeof(Txn));
    if (!txn) {
        perror("malloc failed for Txn");
        return NULL;
    }
    txn->root = t->root;
    txn->size = t->size;
    return txn;
}

// The new transaction contains the current state of the original,
// and further mutations on either transaction will be independent.
Txn *Txn_Clone(Txn *t) {
    Txn *txn = (Txn *)RedisModule_Alloc(sizeof(Txn));
    if (!txn) {
        perror("malloc failed for Txn clone");
        return NULL;
    }
    txn->root = t->root;
    txn->size = t->size;
    return txn;
}

int Txn_trackChannelsAndCount(Txn *t, Node *n) {
    int leaves = 0;
    if (n->leaf != NULL) {
        leaves = 1;
    }
    // Iterate over each edge in n->edges.
    for (size_t i = 0; i < n->edges.len; i++) {
        leaves += Txn_trackChannelsAndCount(t, n->edges.data[i].node);
    }
    return leaves;
}

// A helper function that concatenates two byte arrays.
// It takes pointers to the arrays a and b along with their lengths,
// and writes the new length into out_len.
// The caller is responsible for freeing the returned buffer.
unsigned char *concat(const unsigned char *a, size_t a_len,
                      const unsigned char *b, size_t b_len,
                      size_t *out_len) {
    *out_len = a_len + b_len;
    unsigned char *c = (unsigned char *)RedisModule_Alloc(*out_len);
    if (c == NULL) {
        perror("malloc failed in concat");
        exit(EXIT_FAILURE);
    }
    memcpy(c, a, a_len);
    memcpy(c + a_len, b, b_len);
    return c;
}

// mergeChild collapses node n with its single child.
void Txn_mergeChild(Txn *t, Node *n) {
    if (n->edges.len == 0) {
        // No child edge exists; nothing to merge.
        return;
    }

    // Get the first (and only) edge.
    edge e = n->edges.data[0];
    Node *child = e.node;

    // Merge the prefixes: new_prefix = concat(n->prefix, child->prefix)
    size_t new_prefix_len;
    unsigned char *new_prefix = concat(n->prefix, n->prefix_len,
                                       child->prefix, child->prefix_len,
                                       &new_prefix_len);
    // Free the old prefix if dynamically allocated.
    free(n->prefix);
    n->prefix = new_prefix;
    n->prefix_len = new_prefix_len;

    // Merge the leaf pointers.
    n->leaf = child->leaf;
    n->minLeaf = child->leaf;

    // Merge the edges.
    if (child->edges.len != 0) {
        // Allocate new memory for n->edges.data with the size of child->edges.
        edge *new_edges = (edge *)RedisModule_Alloc(child->edges.len * sizeof(edge));
        if (new_edges == NULL) {
            perror("malloc failed in mergeChild for edges");
            exit(EXIT_FAILURE);
        }
        memcpy(new_edges, child->edges.data, child->edges.len * sizeof(edge));
        n->edges.data = new_edges;
        n->edges.len = child->edges.len;
        n->edges.cap = child->edges.len;
    } else {
        n->edges.data = NULL;
        n->edges.len = 0;
        n->edges.cap = 0;
    }
}

// Txn_insert recursively inserts key/value pair into the tree.
// Parameters:
//   t        - the current transaction
//   n        - current node in the recursion
//   k        - the complete key to insert (pointer)
//   k_len    - length of key k
//   search   - pointer to current search portion of key
//   search_len - length of the search portion
//   v        - value to associate with key k
//   oldVal   - output parameter; on return, set to previous value if updated, or NULL
//   didUpdate- output parameter; nonzero if a value was replaced, zero otherwise
//
// Returns the (possibly updated) node pointer.
Node *Txn_insert(Txn *t, Node *n,
                 unsigned char *k, size_t k_len,
                 unsigned char *search, size_t search_len,
                 void *v, void **oldVal, int *didUpdate) {
    // Handle key exhaustion.
    if (search_len == 0) {
        *oldVal = NULL;
        *didUpdate = 0;
        if (n->leaf != NULL) {
            *oldVal = n->leaf->val;
            *didUpdate = 1;
        }
        // Create a new leaf.
        LeafNode *newLeaf = (LeafNode *)RedisModule_Alloc(sizeof(LeafNode));
        if (!newLeaf) { perror("malloc failed"); exit(EXIT_FAILURE); }
        newLeaf->key = k;
        newLeaf->key_len = k_len;
        newLeaf->val = v;
        newLeaf->nextLeaf = NULL;
        newLeaf->prevLeaf = NULL;
        n->leaf = newLeaf;
        Node_computeLinks(n);
        return n;
    }

    // Look for the edge corresponding to the first byte of search.
    int idx;
    Node *child = Node_getEdge(n, search[0], &idx);

    // No edge found; create one.
    if (child == NULL) {
        // Create a new leaf node.
        LeafNode *leaf = (LeafNode *)RedisModule_Alloc(sizeof(LeafNode));
        if (!leaf) { perror("malloc failed"); exit(EXIT_FAILURE); }
        leaf->key = k;
        leaf->key_len = k_len;
        leaf->val = v;
        leaf->nextLeaf = NULL;
        leaf->prevLeaf = NULL;

        // Create a new Node to hold the leaf.
        Node *newNode = (Node *)RedisModule_Alloc(sizeof(Node));
        if (!newNode) { perror("malloc failed"); exit(EXIT_FAILURE); }
        newNode->leaf = leaf;
        newNode->minLeaf = leaf;
        newNode->maxLeaf = leaf;
        // For the prefix, we use the current search portion.
        newNode->prefix = search;         // pointer assignment; adjust if a copy is needed.
        newNode->prefix_len = search_len;
        // Initialize edges to empty.
        newNode->edges.data = NULL;
        newNode->edges.len = 0;
        newNode->edges.cap = 0;

        // Create an edge for the new node.
        edge e;
        e.label = search[0];
        e.node = newNode;
        Node_addEdge(n, e);
        Node_computeLinks(n);
        *oldVal = NULL;
        *didUpdate = 0;
        return n;
    }

    // Determine the longest common prefix length between search and child's prefix.
    int commonPrefix = longestPrefix(search, search_len, child->prefix, child->prefix_len);
    if (commonPrefix == (int)child->prefix_len) {
        // Consume the common prefix.
        search += commonPrefix;
        search_len -= commonPrefix;
        Node *newChild = Txn_insert(t, child, k, k_len, search, search_len, v, oldVal, didUpdate);
        if (newChild != NULL) {
            n->edges.data[idx].node = newChild;
            Node_computeLinks(n);
            return n;
        }
        return NULL;  // Propagate failure.
    }

    // Split the node.
    // Create a new split node whose prefix is search[0:commonPrefix].
    Node *splitNode = (Node *)RedisModule_Alloc(sizeof(Node));
    if (!splitNode) { perror("malloc failed"); exit(EXIT_FAILURE); }
    splitNode->prefix = (unsigned char *)RedisModule_Alloc(commonPrefix);
    if (!splitNode->prefix) { perror("malloc failed"); exit(EXIT_FAILURE); }
    memcpy(splitNode->prefix, search, commonPrefix);
    splitNode->prefix_len = commonPrefix;
    // Initialize leaf and edges for splitNode.
    splitNode->leaf = NULL;
    splitNode->minLeaf = NULL;
    splitNode->maxLeaf = NULL;
    splitNode->edges.data = NULL;
    splitNode->edges.len = 0;
    splitNode->edges.cap = 0;

    // Replace the existing edge in n with one pointing to splitNode.
    {
        edge newEdge;
        newEdge.label = search[0];
        newEdge.node = splitNode;
        Node_replaceEdge(n, newEdge);
    }

    // Add the existing child as an edge to splitNode.
    {
        edge childEdge;
        childEdge.label = child->prefix[commonPrefix];
        childEdge.node = child;
        Node_addEdge(splitNode, childEdge);
    }
    // Adjust the child's prefix: remove the common prefix.
    {
        size_t newChildPrefixLen = child->prefix_len - commonPrefix;
        unsigned char *newChildPrefix = (unsigned char *)RedisModule_Alloc(newChildPrefixLen);
        if (!newChildPrefix) { perror("malloc failed"); exit(EXIT_FAILURE); }
        memcpy(newChildPrefix, child->prefix + commonPrefix, newChildPrefixLen);
        child->prefix = newChildPrefix;
        child->prefix_len = newChildPrefixLen;
    }

    // Create a new leaf node.
    LeafNode *leaf = (LeafNode *)RedisModule_Alloc(sizeof(LeafNode));
    if (!leaf) { perror("malloc failed"); exit(EXIT_FAILURE); }
    leaf->key = k;
    leaf->key_len = k_len;
    leaf->val = v;
    leaf->nextLeaf = NULL;
    leaf->prevLeaf = NULL;

    // Consume the common prefix from search.
    search += commonPrefix;
    search_len -= commonPrefix;
    if (search_len == 0) {
        splitNode->leaf = leaf;
        splitNode->minLeaf = leaf;
        splitNode->maxLeaf = leaf;
        Node_computeLinks(splitNode);
        Node_computeLinks(n);
        *oldVal = NULL;
        *didUpdate = 0;
        return n;
    }

    // Create a new node to hold the new leaf.
    Node *newLeafNode = (Node *)RedisModule_Alloc(sizeof(Node));
    if (!newLeafNode) { perror("malloc failed"); exit(EXIT_FAILURE); }
    newLeafNode->leaf = leaf;
    newLeafNode->minLeaf = leaf;
    newLeafNode->maxLeaf = leaf;
    newLeafNode->prefix = search;  // use the remaining search as prefix
    newLeafNode->prefix_len = search_len;
    newLeafNode->edges.data = NULL;
    newLeafNode->edges.len = 0;
    newLeafNode->edges.cap = 0;

    {
        edge newLeafEdge;
        newLeafEdge.label = search[0];
        newLeafEdge.node = newLeafNode;
        Node_addEdge(splitNode, newLeafEdge);
    }
    Node_computeLinks(splitNode);
    Node_computeLinks(n);
    *oldVal = NULL;
    *didUpdate = 0;
    return n;
}


// Txn_delete recursively deletes a key from the tree.
// Parameters:
//   t           - pointer to the transaction
//   parent      - pointer to the parent node (may be NULL at the root)
//   n           - current node in recursion
//   search      - pointer to the current portion of the key to delete
//   search_len  - length of the search portion
//
// Returns a DeleteResult containing the updated node and the deleted leaf (if any).
DeleteResult Txn_delete(Txn *t, Node *parent, Node *n, unsigned char *search, size_t search_len) {
    DeleteResult result = { NULL, NULL };

    // Check for key exhaustion.
    if (search_len == 0) {
        if (!Node_isLeaf(n)) {
            return result; // Not a leaf: key not present.
        }
        // Save the old leaf pointer.
        LeafNode *oldLeaf = n->leaf;

        // Remove the leaf.
        n->leaf = NULL;
        n->minLeaf = NULL;
        n->maxLeaf = NULL;

        // If this is not the root and the node has only one edge, merge with its child.
        if (n != t->root && n->edges.len == 1) {
            Txn_mergeChild(t, n);
        }
        result.node = n;
        result.leaf = oldLeaf;
        return result;
    }

    // Look for an edge with the label equal to the first byte of search.
    unsigned char label = search[0];
    int idx;
    Node *child = Node_getEdge(n, label, &idx);
    // Check if the child exists and that its prefix matches the beginning of 'search'.
    if (child == NULL ||
        search_len < child->prefix_len ||
        memcmp(search, child->prefix, child->prefix_len) != 0) {
        return result; // Key not found.
    }

    // Consume the child's prefix from the search key.
    search += child->prefix_len;
    search_len -= child->prefix_len;

    // Recurse into the child.
    DeleteResult subRes = Txn_delete(t, n, child, search, search_len);
    if (subRes.node == NULL) {
        return result; // Deletion failed in subtree.
    }

    // If the updated child has no leaf and no edges, delete this edge.
    if (subRes.node->leaf == NULL && subRes.node->edges.len == 0) {
        Node_delEdge(n, label);
        if (n != t->root && n->edges.len == 1 && !Node_isLeaf(n)) {
            Txn_mergeChild(t, n);
        }
    } else {
        // Otherwise, update the pointer for the edge.
        n->edges.data[idx].node = subRes.node;
    }
    Node_computeLinks(n);
    result.node = n;
    result.leaf = subRes.leaf;
    return result;
}


// Txn_deletePrefix recursively deletes all nodes under a given prefix.
// Parameters:
//   t         - transaction pointer
//   n         - current node
//   search    - pointer to the current portion of the prefix to delete
//   search_len- length of the search portion
// Returns a DeletePrefixResult containing the updated node and the number of deletions.
DeletePrefixResult Txn_deletePrefix(Txn *t, Node *n, unsigned char *search, size_t search_len) {
    DeletePrefixResult result = { NULL, 0 };

    // Check for key exhaustion.
    if (search_len == 0) {
        int delSize = Txn_trackChannelsAndCount(t, n);
        if (Node_isLeaf(n)) {
            n->leaf = NULL;
        }
        // In Go, n.edges = nil; here we reset the edge count.
        n->edges.len = 0;
        // Optionally, free n->edges.data and set it to NULL.
        Node_computeLinks(n);
        result.node = n;
        result.numDeletions = delSize;
        return result;
    }

    // Look for an edge with the first byte of search.
    unsigned char label = search[0];
    int idx;
    Node *child = Node_getEdge(n, label, &idx);
    // Check if the child exists and that either:
    //   (a) child's prefix is a prefix of search, or
    //   (b) search is a prefix of child's prefix.
    bool childPrefixIsPrefix = (child != NULL &&
                                child->prefix_len <= search_len &&
                                memcmp(search, child->prefix, child->prefix_len) == 0);
    bool searchIsPrefixOfChild = (child != NULL &&
                                  search_len < child->prefix_len &&
                                  memcmp(child->prefix, search, search_len) == 0);
    if (child == NULL || (!childPrefixIsPrefix && !searchIsPrefixOfChild)) {
        result.node = NULL;
        result.numDeletions = 0;
        return result;
    }

    // Consume the child's prefix from search.
    if (child->prefix_len > search_len) {
        // If child's prefix is longer than search, set search to empty.
        search = (unsigned char *)"";
        search_len = 0;
    } else {
        search += child->prefix_len;
        search_len -= child->prefix_len;
    }

    // Recurse into the child.
    DeletePrefixResult subRes = Txn_deletePrefix(t, child, search, search_len);
    if (subRes.node == NULL) {
        result.node = NULL;
        result.numDeletions = 0;
        return result;
    }

    // If the updated child has no leaf and no edges, delete this edge.
    if (subRes.node->leaf == NULL && subRes.node->edges.len == 0) {
        Node_delEdge(n, label);
        if (n != t->root && n->edges.len == 1 && !Node_isLeaf(n)) {
            Txn_mergeChild(t, n);
        }
    } else {
        n->edges.data[idx].node = subRes.node;
    }
    Node_computeLinks(n);
    result.node = n;
    result.numDeletions = subRes.numDeletions;
    return result;
}

InsertResult Txn_Insert(Txn *t, unsigned char *k, size_t k_len, void *v) {
    InsertResult res = { NULL, 0 };
    // Call the recursive insert function.
    // We pass k as both the full key and the search key.
    Node *newRoot = Txn_insert(t, t->root, k, k_len, k, k_len, v, &(res.oldVal), &(res.didUpdate));
    if (newRoot != NULL) {
        t->root = newRoot;
    }
    // If no update occurred, increment the tree size.
    if (!res.didUpdate) {
        t->size++;
    }
    return res;
}

DeleteReturn Txn_Delete(Txn *t, unsigned char *k, size_t k_len) {
    DeleteResult dr = Txn_delete(t, NULL, t->root, k, k_len);
    t->root = dr.node;
    // If the tree becomes empty, allocate a new empty node.
    if (t->root == NULL) {
        t->root = (Node *)RedisModule_Alloc(sizeof(Node));
        if (!t->root) {
            perror("malloc failed for new Node");
            exit(EXIT_FAILURE);
        }
        // Initialize the new Node appropriately.
        t->root->leaf = NULL;
        t->root->minLeaf = NULL;
        t->root->maxLeaf = NULL;
        t->root->prefix = NULL;
        t->root->prefix_len = 0;
        t->root->edges.data = NULL;
        t->root->edges.len = 0;
        t->root->edges.cap = 0;
    }

    DeleteReturn ret;
    if (dr.leaf != NULL) {
        t->size--;
        ret.oldVal = dr.leaf->val;
        ret.found = 1;
    } else {
        ret.oldVal = NULL;
        ret.found = 0;
    }
    return ret;
}

// Txn_DeletePrefix deletes the subtree matching the given prefix.
// It updates the transaction's root and size, and if the root becomes NULL,
// a new empty Node is allocated.
DeletePrefixReturn Txn_DeletePrefix(Txn *t, unsigned char *prefix, size_t prefix_len) {
    DeletePrefixResult dpr = Txn_deletePrefix(t, t->root, prefix, prefix_len);
    t->root = dpr.node;
    t->size -= dpr.numDeletions;
    if (t->root == NULL) {
        // Allocate a new empty node.
        t->root = (Node *)RedisModule_Alloc(sizeof(Node));
        if (!t->root) {
            perror("malloc failed for new Node");
            exit(EXIT_FAILURE);
        }
        t->root->leaf = NULL;
        t->root->minLeaf = NULL;
        t->root->maxLeaf = NULL;
        t->root->prefix = NULL;
        t->root->prefix_len = 0;
        t->root->edges.data = NULL;
        t->root->edges.len = 0;
        t->root->edges.cap = 0;
    }
    DeletePrefixReturn ret;
    ret.success = 1; // Always true in this implementation.
    ret.numDeletions = dpr.numDeletions;
    return ret;
}

Node *Txn_Root(Txn *t) {
    return t->root;
}

void *Txn_Get(Txn *t, unsigned char *k, size_t k_len, int *found) {
    return Node_Get(t->root, k, k_len, found);
}

Tree *Txn_Commit(Txn *t) {
    Tree *nt = Txn_CommitOnly(t);
    return nt;
}


Tree *Txn_CommitOnly(Txn *t) {
    Tree *nt = (Tree *)RedisModule_Alloc(sizeof(Tree));
    if (!nt) {
        perror("malloc failed");
        exit(EXIT_FAILURE);
    }
    nt->root = t->root;
    nt->size = t->size;
    return nt;
}


// Tree_Insert starts a new transaction, inserts key/value pair, and commits the transaction.
// Parameters:
//   t       - pointer to the current tree
//   k       - key to insert (byte array)
//   k_len   - length of the key
//   v       - value to insert (void pointer)
//   oldVal  - output parameter to receive the old value (if any)
//   didUpdate - output parameter: nonzero if the key was updated, zero if inserted.
Tree *Tree_Insert(Tree *t, unsigned char *k, size_t k_len, void *v, void **oldVal, int *didUpdate) {
    Txn *txn = Tree_Txn(t);
    InsertResult res = Txn_Insert(txn, k, k_len, v);
    Tree *newTree = Txn_Commit(txn);
    *oldVal = res.oldVal;
    *didUpdate = res.didUpdate;
    return newTree;
}

// Tree_Delete starts a transaction to delete a key, commits the transaction,
// and returns the new tree along with the old value (if any) and a flag indicating deletion.
Tree *Tree_Delete(Tree *t, unsigned char *k, size_t k_len, void **oldVal, int *found) {
    Txn *txn = Tree_Txn(t);
    DeleteReturn dr = Txn_Delete(txn, k, k_len);
    Tree *newTree = Txn_Commit(txn);
    *oldVal = dr.oldVal;
    *found = dr.found;
    return newTree;
}

// Tree_DeletePrefix starts a transaction to delete all nodes starting with a given prefix,
// commits the transaction, and returns the new tree along with a success flag and the number of deletions.
Tree *Tree_DeletePrefix(Tree *t, unsigned char *k, size_t k_len, int *success, int *numDel) {
    Txn *txn = Tree_Txn(t);
    DeletePrefixReturn dpr = Txn_DeletePrefix(txn, k, k_len);
    Tree *newTree = Txn_Commit(txn);
    *success = dpr.success;
    *numDel = dpr.numDeletions;
    return newTree;
}

Node *Tree_Root(Tree *t) {
    return t->root;
}

// Looks up a specific key in the tree.
// The found flag is set to nonzero if the key is found.
void *Tree_Get(Tree *t, unsigned char *k, size_t k_len, int *found) {
    return Node_Get(t->root, k, k_len, found);
}

// longestPrefix finds the length of the shared prefix of two byte arrays.
int longestPrefix(const unsigned char *k1, size_t k1_len,
                  const unsigned char *k2, size_t k2_len) {
    size_t max = k1_len;
    if (k2_len < max) {
        max = k2_len;
    }
    int i;
    for (i = 0; i < (int)max; i++) {
        if (k1[i] != k2[i]) {
            break;
        }
    }
    return i;
}
