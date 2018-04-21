#include "ref_trie.h"
#include <stdlib.h>

void libab_ref_trie_init(libab_ref_trie* trie) {
    libab_ref_null(&trie->null_ref);
    trie->head = NULL;
}

libab_result _libab_ref_trie_put(libab_ref_trie_node** node, const char* key,
                                 libab_ref* ref) {
    libab_result result = LIBAB_SUCCESS;
    if ((*node = malloc(sizeof(**node)))) {
        (*node)->key = *key;
        (*node)->next = NULL;

        if (*(key + 1)) {
            libab_ref_null(&(*node)->ref);
            result = _libab_ref_trie_put(&(*node)->child, key + 1, ref);
        } else {
            libab_ref_copy(ref, &(*node)->ref);
            (*node)->child = NULL;
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        if (*node)
            libab_ref_free(&(*node)->ref);
        free(*node);
        *node = NULL;
    }

    return result;
}

libab_result libab_ref_trie_put(libab_ref_trie* trie, const char* key,
                                libab_ref* ref) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref_trie_node** current = &trie->head;
    char search;
    while (*key) {
        search = *key;
        while (*current && (*current)->key != search) {
            current = &(*current)->next;
        }
        if (*current) {
            if (*(key + 1)) {
                current = &(*current)->child;
            } else {
                libab_ref_free(&(*current)->ref);
                libab_ref_copy(ref, &(*current)->ref);
            }
            key++;
        } else {
            result = _libab_ref_trie_put(current, key, ref);
            break;
        }
    }
    return result;
}

const libab_ref* libab_ref_trie_get(const libab_ref_trie* trie,
                                    const char* key) {
    libab_ref_trie_node* current = trie->head;
    while (current && *key) {
        while (current && current->key != *key) {
            current = current->next;
        }
        if (current == NULL)
            break;

        if (*(key + 1)) {
            current = current->child;
            key++;
        } else {
            return &current->ref;
        }
    }
    return &trie->null_ref;
}

void _libab_ref_trie_free(libab_ref_trie_node* node) {
    if (node == NULL)
        return;
    _libab_ref_trie_free(node->next);
    _libab_ref_trie_free(node->child);
    libab_ref_free(&node->ref);
    free(node);
}

void libab_ref_trie_free(libab_ref_trie* trie) {
    _libab_ref_trie_free(trie->head);
    libab_ref_free(&trie->null_ref);
}
