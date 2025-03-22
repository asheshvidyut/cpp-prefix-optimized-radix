//
// Created by Ashesh Vidyut on 22/03/25.
//

#ifndef RADIX_H
#define RADIX_H

#include "node.hpp"    // Declaration of Node, LeafNode, and edge types
#include <stdbool.h>

// ----------- Tree and Transaction Types -----------
// Forward declaration of Node.
typedef struct Node Node;
typedef struct LeafNode LeafNode;

typedef struct Tree {
    Node *root;
    int size;
} Tree;

typedef struct Txn {
    Node *root;
    int size;
} Txn;

// A structure to return both an updated node and a deleted leaf.
typedef struct {
    Node *node;      // updated node pointer (NULL if deletion failed)
    LeafNode *leaf;  // the deleted leaf (if any)
} DeleteResult;

// A structure to hold the result of deletePrefix.
typedef struct {
    Node *node;      // The updated node pointer; NULL if deletion failed.
    int numDeletions; // Number of deletions in the subtree.
} DeletePrefixResult;

typedef struct {
    void *oldVal;   // previous value (if any)
    int didUpdate;  // nonzero if the key was updated; 0 otherwise
} InsertResult;

typedef struct {
    void *oldVal;  // previous value stored under the key, if any
    int found;     // nonzero if the key was found and deleted, zero otherwise
} DeleteReturn;

// Structure returned by the Txn_DeletePrefix function.
typedef struct {
    int success;      // 1 if deletion occurred successfully.
    int numDeletions; // Number of deletions.
} DeletePrefixReturn;

// ----------- Tree Function Prototypes -----------

// Declarations like:
Tree *Tree_New(void);
Node *Tree_Root(Tree *);

// Returns the number of elements in the tree.
int Tree_Len(Tree *t);

// Starts a new transaction on the tree.
Txn *Tree_Txn(Tree *t);

// Inserts a key/value pair into the transaction.
// Returns: new root node, pointer to previous value (if any) via oldVal, and didUpdate flag (nonzero if updated).
Node *Txn_insert(Txn *t, Node *n,
                 unsigned char *k, size_t k_len,
                 unsigned char *search, size_t search_len,
                 void *v, void **oldVal, int *didUpdate);

// Deletes a key from the tree recursively.
// Returns: new root node and pointer to deleted leaf (if any) via oldLeaf.
DeleteResult Txn_delete(Txn *t, Node *parent, Node *n, unsigned char *search, size_t search_len);

// Deletes all nodes under a given prefix.
// Returns: new root node and number of deletions via numDeletions.
DeletePrefixResult Txn_deletePrefix(Txn *t, Node *n, unsigned char *search, size_t search_len);

// Merges the given node with its single child.
void Txn_mergeChild(Txn *t, Node *n);

// Traverses the subtree at n, tracking channels and counting leaves.
// Returns the number of leaves in the subtree.
int Txn_trackChannelsAndCount(Txn *t, Node *n);

// Clones the transaction.
Txn *Txn_Clone(Txn *t);

// Commits the transaction and returns a new Tree.
Tree *Txn_Commit(Txn *t);

// Commits the transaction without issuing notifications.
Tree *Txn_CommitOnly(Txn *t);

// Inserts a key/value pair into the tree (transactionally).
// Returns the new tree, previous value via oldVal, and didUpdate flag.
Tree *Tree_Insert(Tree *, unsigned char *, size_t , void *, void **, int *);

// Deletes a key from the tree.
// Returns the new tree, the old value via oldVal, and a flag indicating if deletion occurred.
Tree *Tree_Delete(Tree *t, unsigned char *k, size_t k_len, void **oldVal, int *found);

// Deletes all nodes starting with the given prefix.
// Returns the new tree, a flag indicating if any deletion occurred, and number of deletions.
Tree *Tree_DeletePrefix(Tree *t, unsigned char *k, size_t k_len, int *success, int *numDel);

// Looks up a key in the tree.
// Returns the associated value and sets found to nonzero if found.
void *Tree_Get(Tree *t, unsigned char *k, size_t k_len, int *found);

// ----------- Helper Functions -----------

// Returns the length of the longest common prefix between two byte arrays.
int longestPrefix(const unsigned char *k1, size_t k1_len, const unsigned char *k2, size_t k2_len);

// Concatenates two byte arrays into a newly allocated array.
// The caller is responsible for freeing the returned array.
unsigned char *concat(const unsigned char *a, size_t a_len, const unsigned char *b, size_t b_len, size_t* out_len);

#endif //RADIX_H
