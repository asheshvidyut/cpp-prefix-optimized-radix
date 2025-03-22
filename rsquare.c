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

static Tree *tree = NULL;

static RedisModuleCtx *globalCtx = NULL;

void initRadixTree() {
    tree = Tree_New();
}

// Placeholder: update your radix tree with the given key.
void insertInRadixTree(const char *key) {
    // Insert or update the key in your radix tree.
    // For example: radix_tree_insert(myRadixTree, key);
    void * oldVal;
    int * didUpdate;
    tree = Tree_Insert(tree, key, strlen(key), NULL, &oldVal, &didUpdate);
}

void deleteInRadixTree(const char *key) {
    // Insert or update the key in your radix tree.
    // For example: radix_tree_insert(myRadixTree, key);
    int found;
    tree = Tree_Delete(tree, key, strlen(key), NULL, &found);
}

// This function scans the entire keyspace and updates the radix tree.
int updateRadixTreeForAllKeys(RedisModuleCtx *ctx) {
    // Start with cursor "0".
    char *cursor = strdup("0");
    if (cursor == NULL) return REDISMODULE_ERR;

    do {
        // Execute the SCAN command with a COUNT of 100 (adjust as needed).
        RedisModuleCallReply *reply = RedisModule_Call(ctx, "SCAN", "sc", cursor, "COUNT", "100");
        if (!reply) {
            free(cursor);
            return REDISMODULE_ERR;
        }

        // The reply is an array: [cursor, [keys]]
        RedisModuleCallReply *cursorReply = RedisModule_CallReplyArrayElement(reply, 0);
        size_t cursorLen;
        const char *newCursor = RedisModule_CallReplyStringPtr(cursorReply, &cursorLen);

        // Update the cursor by freeing the old copy and duplicating the new one.
        free(cursor);
        cursor = strdup(newCursor);
        if (cursor == NULL) {
            RedisModule_FreeCallReply(reply);
            return REDISMODULE_ERR;
        }

        // Retrieve the array of keys.
        RedisModuleCallReply *keysArray = RedisModule_CallReplyArrayElement(reply, 1);
        size_t keysCount = RedisModule_CallReplyLength(keysArray);
        for (size_t i = 0; i < keysCount; i++) {
            RedisModuleCallReply *keyReply = RedisModule_CallReplyArrayElement(keysArray, i);
            size_t keyLen;
            const char *keyStr = RedisModule_CallReplyStringPtr(keyReply, &keyLen);
            insertInRadixTree(keyStr);
        }
        RedisModule_FreeCallReply(reply);

    } while (strcmp(cursor, "0") != 0);

    free(cursor);
    return REDISMODULE_OK;
}

int RSquareSearchCommand(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    // Your logic to search the radix tree.
    if (argc != 2) {
        return RedisModule_WrongArity(ctx);
    }

    size_t prefixLen;
    const char *prefix = RedisModule_StringPtrLen(argv[1], &prefixLen);

    char **matches = NULL;  // array of matching strings
    size_t count = 0;       // number of matches stored so far
    size_t capacity = 0;    // current allocated capacity for matches array

    // Assume Tree_SearchPrefix returns a dynamically allocated array of matching keys
    // and sets the count of matches.
    // Get an iterator from the tree's root.
    // Assume Root() returns a pointer to a Node, and that Node has a method Iterator().
    Iterator* iter = Node_Iterator(Tree_Root(tree));
    Iterator_SeekPrefix(iter, prefix, prefixLen);

    // Iterate over the tree, collecting keys into the result vector.
    while (true) {
        IteratorResult result = Iterator_Next(iter);
        if (!result.found) {
            break;
        }
        // Assume Iterator::Next() returns a tuple: (std::string key, void* value, bool found)
        if (count == capacity) {
            capacity = (capacity == 0) ? 16 : capacity * 2;
            char **newMatches = realloc(matches, capacity * sizeof(char *));
            if (newMatches == NULL) {
                // Clean up previously allocated matches if realloc fails.
                for (size_t i = 0; i < count; i++) {
                    free(matches[i]);
                }
                free(matches);
                return RedisModule_ReplyWithError(ctx, "ERR realloc failed");
            }
            matches = newMatches;
        }
        // Duplicate the key string so we can store it.
        // (Assuming result.key is a null-terminated C-string)
        matches[count] = strdup(result.key);
        if (matches[count] == NULL) {
            // Clean up on allocation failure.
            for (size_t i = 0; i < count; i++) {
                free(matches[i]);
            }
            free(matches);
            return RedisModule_ReplyWithError(ctx, "ERR strdup failed");
        }
        count++;
    }

    if (matches == NULL || count == 0) {
        return RedisModule_ReplyWithArray(ctx, 0);
    }

    // Reply with the array of matches.
    RedisModule_ReplyWithArray(ctx, count);
    for (size_t i = 0; i < count; i++) {
        RedisModule_ReplyWithStringBuffer(ctx, matches[i], strlen(matches[i]));
        free(matches[i]);  // Free each match string.
    }
    free(matches); // Free the array pointer.

    return REDISMODULE_OK;
}

// Example command filter function that updates the radix tree when key-creating commands are executed.
static int RSquareFilter(RedisModuleCommandFilterCtx *filter) {
    RedisModule_Log(globalCtx, "debug", "RSquareFilter invoked. Args count: %d", RedisModule_CommandFilterArgsCount(filter));
    int argc = RedisModule_CommandFilterArgsCount(filter);
    if (argc < 2) return REDISMODULE_OK;

    const char *cmd = RedisModule_CommandFilterArgGet(filter, 0);
    // Check for key-creating commands.
    if (strcasecmp(cmd, "SET") == 0) {
        const char *key = RedisModule_CommandFilterArgGet(filter, 1);
        RedisModule_Log(globalCtx, "notice", "Processing SET for key: %s", key);
        insertInRadixTree(key);
    } else if (strcasecmp(cmd, "DEL") == 0) {
        for (int i = 1; i < argc; i++) {
            const char *key = RedisModule_CommandFilterArgGet(filter, i);
            deleteInRadixTree(key);
        }
    } else {

    }
    return REDISMODULE_OK;
}

#ifdef __cplusplus
extern "C" {
#endif

// Module initialization function.
int RedisModule_OnLoad(RedisModuleCtx *ctx, RedisModuleString **argv, int argc) {
    globalCtx = ctx;
    initRadixTree();

    // Initialize the module.
    if (RedisModule_Init(ctx, "rsquare", 1, REDISMODULE_APIVER_1) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    // Register the command filter to update the radix tree for new key creations.
    if (RedisModule_RegisterCommandFilter(ctx, (RedisModuleCommandFilterFunc) RSquareFilter, 0) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    // Register a custom command to query your radix tree.
    if (RedisModule_CreateCommand(ctx, "rsquare.search", RSquareSearchCommand,"readonly", 0, 0, 0) == REDISMODULE_ERR) {
        return REDISMODULE_ERR;
    }

    return REDISMODULE_OK;
}

#ifdef __cplusplus
}
#endif