//
// Created by Ashesh Vidyut on 22/03/25.
//

#include "redismodule.h"
#include<string.h>
#include <stdlib.h>
#include "radix/radix.cpp"
#include "radix/iterator.cpp"
#include "radix/node.cpp"
#include "radix/rev_iterator.cpp"


#ifdef __cplusplus
extern "C" {
#endif

static Tree *tree = NULL;

static RedisModuleCtx *globalCtx = NULL;

static void initRadixTree() {
    tree = Tree_New();
}

// Placeholder: update your radix tree with the given key.
void insertInRadixTree(unsigned char *key, int len) {
    // Insert or update the key in your radix tree.
    // For example: radix_tree_insert(myRadixTree, key);
    void * oldVal;
    int * didUpdate;
    tree = Tree_Insert(tree, key, len, NULL, &oldVal, &didUpdate);
}

void deleteInRadixTree(unsigned char *key, int len) {
    // Insert or update the key in your radix tree.
    // For example: radix_tree_insert(myRadixTree, key);
    int found;
    tree = Tree_Delete(tree, key, len, NULL, &found);
}

int RSquareSearchCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Check for the correct number of arguments.
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    // Retrieve the prefix and its length.
    size_t prefixLen;
    const char *prefix = RedisModule_StringPtrLen(argv[1], &prefixLen);

    printf("Key batman: %s\n", prefix);

    // Create an iterator from the tree's root.
    Iterator *iter = Node_Iterator(Tree_Root(tree));
    if (iter == NULL) {
        return RedisModule_ReplyWithError(ctx, "ERR failed to create iterator");
    }

    // Ensure the radix tree is initialized.
    if (tree == NULL) {
        return RedisModule_ReplyWithError(ctx, "ERR radix tree not initialized");
    }

    // Seek to the first key that matches the given prefix.
    Iterator_SeekPrefix(iter, (const unsigned char *) prefix, (int) prefixLen);

    // Begin an array reply with a postponed length.
    RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
    int count = 0;

    // Iterate over the tree, adding each matching key to the reply.
    while (true) {
        IteratorResult result = Iterator_Next(iter);
        if (!result.found) {
            break;
        }
        printf("Key: %s\n", result.key);
        // Assuming result.key is a null-terminated string and result.key_len contains its length.
        RedisModule_ReplyWithStringBuffer(ctx, (const char *) result.key, result.key_len);
        count++;
    }

    // Set the correct array length in the reply.
    RedisModule_ReplySetArrayLength(ctx, count);

    return REDISMODULE_OK;
}

int RSquareInsertCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Your logic to search the radix tree.
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    size_t keyLen;
    const char *key = RedisModule_StringPtrLen(argv[1], &keyLen);

    printf("Key: %s\n", key);
    insertInRadixTree((const unsigned char *) key, (int) keyLen);
    return RedisModule_ReplyWithSimpleString(ctx, "OK");
}

int RSquareDeleteCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Your logic to search the radix tree.
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }
    RedisModule_AutoMemory(ctx);

    size_t keyLen;
    const char *key = RedisModule_StringPtrLen(argv[1], &keyLen);

    printf("Key: %s\n", key);
    deleteInRadixTree((const unsigned char *) key, (int) keyLen);
    return RedisModule_ReplyWithSimpleString(ctx, "OK");
}

// Module initialization function.
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    globalCtx = ctx;
    initRadixTree();

    // Initialize the module.
    if (RedisModule_Init(ctx, "rsquare", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    // Register a custom command to query your radix tree.
    if (RedisModule_CreateCommand(ctx, "rsquare.search", RSquareSearchCommand,"write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    // Register a custom command to insert your radix tree.
    if (RedisModule_CreateCommand(ctx, "rsquare.insert", RSquareInsertCommand,"write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    // Register a custom command to delete your radix tree.
    if (RedisModule_CreateCommand(ctx, "rsquare.delete", RSquareDeleteCommand,"write", 1, 1, 1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

#ifdef __cplusplus
}
#endif
#pragma clang diagnostic pop