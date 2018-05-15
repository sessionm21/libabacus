#ifndef LIBABACUS_REF_TRIE_H
#define LIBABACUS_REF_TRIE_H

#include "refcount.h"
#include "result.h"

/**
 * A node in the trie.
 * unlike the normal libab_trie,
 * this trie only has at most one value for each key.
 */
struct libab_ref_trie_node_s {
    /**
     * The next child of this trie node, which represents
     * a letter the same distance from the beginning.
     */
    struct libab_ref_trie_node_s* next;
    /**
     * The child of this node, which represents a letter
     * one further away than this node, with the same prefix.
     */
    struct libab_ref_trie_node_s* child;
    /**
     * The key of this node.
     */
    char key;
    /**
     * The reference pointed to by this node.
     */
    libab_ref ref;
};

/**
 * The struct that holds the trie.
 */
struct libab_ref_trie_s {
    /**
     * The first node in the trie.
     */
    struct libab_ref_trie_node_s* head;
};

typedef struct libab_ref_trie_node_s libab_ref_trie_node;
typedef struct libab_ref_trie_s libab_ref_trie;

/**
 * Initializes the trie with no nodes.
 * @param trie the trie to initialize.
 */
void libab_ref_trie_init(libab_ref_trie* trie);
/**
 * Initializes a new trie with a shallow copy of another.
 * @param trie the trie to initialize.
 * @param copy_of the trie to copy.
 * @return the result of the initialization.
 */
libab_result libab_ref_trie_init_copy(libab_ref_trie* trie, 
                                      const libab_ref_trie* copy_of);
/**
 * Stores a reference counted value into the trie.
 * This releases the reference for the given key, if one
 * already exists. This increments the refcoutn.
 * @param trie the trie to insert into.
 * @param key the key to insert under.
 * @param ref the reference to insert.
 * @return the result of the insertion.
 */
libab_result libab_ref_trie_put(libab_ref_trie* trie, const char* key,
                                libab_ref* ref);
/**
 * Retreives a reference under the given key in the trie.
 * @param trie the trie to retreive from.
 * @param key the key to look under.
 * @return a reference stored under the given key. This can be a NULL reference.
 */
void libab_ref_trie_get(const libab_ref_trie* trie,
                                    const char* key, libab_ref* into);
/**
 * Releases the trie, decrementing the refcounts of all
 * the values stored inside.
 * @param trie the trie to release.
 */
void libab_ref_trie_free(libab_ref_trie* trie);

#endif
