#include "trie.h"
#include <stdlib.h>
#include "util.h"

void libab_trie_init(libab_trie* trie) {
    trie->head = NULL;
    ll_init(&trie->empty_list);
}

void _libab_trie_free(libab_trie_node* to_free) {
    if(to_free == NULL) return;
    _libab_trie_free(to_free->next);
    _libab_trie_free(to_free->child);
    ll_free(&to_free->values);
    free(to_free);
}

libab_result _libab_trie_put(libab_trie_node** node, const char* key, void* value) {
    libab_result result = LIBAB_SUCCESS;
    if((*node = malloc(sizeof(**node)))) {
        (*node)->key = *key;
        (*node)->next = NULL;
        ll_init(&(*node)->values);

        if(*(key + 1)) {
            result = _libab_trie_put(&(*node)->child, key + 1, value);
        } else {
            (*node)->child = NULL;
            result = libab_convert_ds_result(ll_append(&(*node)->values, value));
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if(result != LIBAB_SUCCESS) {
        free(*node);
        *node = NULL;
    }

    return result;
}

libab_result _libab_trie_add(libab_trie_node* node, void* value) {
    return libab_convert_ds_result(ll_append(&node->values, value));
}

libab_result libab_trie_put(libab_trie* trie, const char* key, void* value) {
    libab_result result = LIBAB_SUCCESS;
    libab_trie_node** current = &trie->head;
    char search;
    while(*key) {
        search = *key;
        while(*current && (*current)->key != search) {
            current = &(*current)->next;
        }
        if(*current) {
            if(*(key + 1)) {
                current = &(*current)->child;
            } else {
                result = _libab_trie_add(*current, value);
            }
            key++;
        } else {
            result = _libab_trie_put(current, key, value);
            break;
        }
    }
    return result;
}

const ll* libab_trie_get(const libab_trie* trie, const char* key) {
    libab_trie_node* current = trie->head;
    while(current && *key) {
        while(current && current->key != *key) {
            current = current->next;
        }
        if(current == NULL) break;
        
        if(*(key + 1)) {
            current = current->child;
            key++;
        } else {
            return &current->values;
        }
    }
    return &trie->empty_list;
}

int _libab_trie_foreach(libab_trie_node* node, void* data, compare_func compare, foreach_func foreach) {
    int return_code;
    if(node == NULL) return 0;
    return_code = ll_foreach(&node->values, data, compare, foreach);
    if(return_code == 0) {
        return_code = _libab_trie_foreach(node->child, data, compare, foreach);  
    }
    if(return_code == 0) {
        return_code = _libab_trie_foreach(node->next, data, compare, foreach);
    }
    return return_code;
}

int libab_trie_foreach(const libab_trie* trie, void* data, compare_func compare, foreach_func foreach) {
    return _libab_trie_foreach(trie->head, data, compare, foreach);
}

void libab_trie_free(libab_trie* trie) {
    _libab_trie_free(trie->head);   
    trie->head = NULL;
}
