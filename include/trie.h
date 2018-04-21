#ifndef LIBABACUS_TRIE_H
#define LIBABACUS_TRIE_H

#include "ll.h"
#include "result.h"

/**
 * A node in the trie.
 */
struct libab_trie_node_s {
    /**
     * The "child" node, which points
     * to the next layer of the trie, housing
     * children that have a prefix ending in this node's key value.
     */
    struct libab_trie_node_s* child;
    /**
     * The "next" node, which points
     * to the sibiling of this tree, wich has the same prefix
     * as this node.
     */
    struct libab_trie_node_s* next;
    /**
     * The values associated with the key ending with
     * this node's key value.
     */
    ll values;
    /**
     * The last letter of the key, which this node represents.
     * The remainder of the key is encoded in nodes preceding this one.
     */
    char key;
};

/**
 * A struct that represents a trie.
 */
struct libab_trie_s {
    /**
     * The first search node in this trie.
     */
    struct libab_trie_node_s* head;
    /**
     * The empty list returned if no value is found.
     * Note that existing nodes return their own linked
     * list of values, even if empty. However, for keys
     * that don't exist as prefixes in the trie,
     * this list is returned to maintain consistency:
     * a list is always returned containing the values
     * of the trie associated with the given key.
     */
    ll empty_list;
};

typedef struct libab_trie_s libab_trie;
typedef struct libab_trie_node_s libab_trie_node;

void libab_trie_init(libab_trie* trie);
libab_result libab_trie_put(libab_trie* trie, const char* key, void* value);
const ll* libab_trie_get(const libab_trie* trie, const char* key);
int libab_trie_foreach(const libab_trie* trie, void* data, compare_func compare,
                       foreach_func foreach);
void libab_trie_free(libab_trie* trie);

#endif
