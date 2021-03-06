#include "ref_trie.h"
#include <stdlib.h>

void libab_ref_trie_init(libab_ref_trie* trie) { trie->head = NULL; }

void _libab_ref_trie_free(libab_ref_trie_node* node) {
    if (node == NULL)
        return;
    _libab_ref_trie_free(node->next);
    _libab_ref_trie_free(node->child);
    libab_ref_free(&node->ref);
    free(node);
}

libab_result _libab_ref_trie_copy(const libab_ref_trie_node* copy_of,
                                  libab_ref_trie_node** copy_into) {
    libab_result result = LIBAB_SUCCESS;

    if (copy_of == NULL) {
        *copy_into = NULL;
    } else if (((*copy_into) = malloc(sizeof(**copy_into)))) {
        (*copy_into)->child = NULL;
        (*copy_into)->next = NULL;

        result = _libab_ref_trie_copy(copy_of->next, &(*copy_into)->next);
        if (result == LIBAB_SUCCESS) {
            result = _libab_ref_trie_copy(copy_of->child, &(*copy_into)->child);
        }

        if (result == LIBAB_SUCCESS) {
            (*copy_into)->key = copy_of->key;
            libab_ref_copy(&copy_of->ref, &(*copy_into)->ref);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS && *copy_into) {
        _libab_ref_trie_free((*copy_into)->next);
        _libab_ref_trie_free((*copy_into)->child);
        free(*copy_into);
        *copy_into = NULL;
    }

    return result;
}

libab_result libab_ref_trie_init_copy(libab_ref_trie* trie,
                                      const libab_ref_trie* copy_of) {
    libab_result result = LIBAB_SUCCESS;

    libab_ref_trie_init(trie);
    result = _libab_ref_trie_copy(copy_of->head, &trie->head);

    if (result != LIBAB_SUCCESS) {
        libab_ref_trie_free(trie);
    }

    return result;
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

void libab_ref_trie_get(const libab_ref_trie* trie, const char* key,
                        libab_ref* into) {
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
            libab_ref_copy(&current->ref, into);
            return;
        }
    }
    libab_ref_null(into);
}

libab_result _ref_trie_foreach(libab_ref_trie_node* node,
                               char** str, size_t* str_size, size_t depth,
                               libab_result (*func)(const libab_ref*, const char*, va_list),
                               va_list args) {
    va_list args_copy;
    libab_result result = LIBAB_SUCCESS;
    if(node == NULL) {
        return result;
    }

    if(depth + 1 >= *str_size) {
        char* new_str = realloc(*str, (*str_size) *= 2);
        if(new_str) {
            *str = new_str;
        } else {
            result = LIBAB_MALLOC;
        }
    }

    if (result == LIBAB_SUCCESS) {
        (*str)[depth] = node->key;
        (*str)[depth + 1] = '\0';

        if(libab_ref_get(&node->ref)) {
            va_copy(args_copy, args);
            result = func(&node->ref, (*str), args_copy);
            va_end(args_copy);
        }

        if(result == LIBAB_SUCCESS)
            result = _ref_trie_foreach(node->child, str, str_size, depth + 1, func, args);
        if(result == LIBAB_SUCCESS)
            result = _ref_trie_foreach(node->next, str, str_size, depth, func, args);
    }

    return result;
}

libab_result libab_ref_trie_foreach(const libab_ref_trie* trie,
                            libab_result (*func)(const libab_ref*, const char*, va_list),
                            ...) {
    va_list args;
    libab_result result = LIBAB_SUCCESS;
    char* str;
    size_t string_size = 4;

    if((str = malloc(sizeof(*str) * 4))) {
        va_start(args, func);
        result = _ref_trie_foreach(trie->head, &str, &string_size, 0, func, args);
        va_end(args);
    } else {
        result = LIBAB_MALLOC;
    }

    free(str);

    return result;
}

void libab_ref_trie_free(libab_ref_trie* trie) {
    _libab_ref_trie_free(trie->head);
}
