#include "tree.h"
#include <stdlib.h>

int libab_tree_has_vector(libab_tree_variant variant) {
    return variant == TREE_BASE || variant == TREE_OP ||
           variant == TREE_PREFIX_OP || variant == TREE_POSTFIX_OP ||
           variant == TREE_BLOCK || variant == TREE_IF ||
           variant == TREE_CALL || variant == TREE_WHILE ||
           variant == TREE_DOWHILE || variant == TREE_FUN ||
           variant == TREE_RETURN || variant == TREE_RESERVED_OP;
}

int libab_tree_has_string(libab_tree_variant variant) {
    return variant == TREE_ID || variant == TREE_NUM || variant == TREE_OP ||
           variant == TREE_PREFIX_OP || variant == TREE_POSTFIX_OP ||
           variant == TREE_FUN || variant == TREE_FUN_PARAM ||
           variant == TREE_RESERVED_OP;
}

int libab_tree_has_scope(libab_tree_variant variant) {
    return variant == TREE_BASE || variant == TREE_BLOCK ||
           variant == TREE_IF || variant == TREE_WHILE ||
           variant == TREE_DOWHILE;
}

int libab_tree_has_type(libab_tree_variant variant) {
    return variant == TREE_FUN_PARAM || variant == TREE_FUN;
}

void libab_tree_free(libab_tree* tree) {
    int free_string = 0;
    int free_vector = 0;
    int free_type = 0;
    free_vector = libab_tree_has_vector(tree->variant);
    free_string = libab_tree_has_string(tree->variant);
    free_type = libab_tree_has_type(tree->variant);
    if (free_string)
        free(tree->string_value);
    if (free_vector)
        vec_free(&tree->children);
    if (free_type)
        libab_ref_free(&tree->type);
}

int _tree_foreach_free(void* data, va_list args) {
    libab_tree_free_recursive(data);
    return 0;
}

int _tree_needs_free(libab_tree* tree) {
    return ((tree->variant == TREE_FUN && --(tree->int_value) == 0) |
            (tree->variant != TREE_FUN));
}

void libab_tree_free_recursive(libab_tree* tree) {
    if(_tree_needs_free(tree)) {
        if (libab_tree_has_vector(tree->variant)) {
            vec_foreach(&tree->children, NULL, compare_always, _tree_foreach_free);
        }
        libab_tree_free(tree);
        free(tree);
    }
}

void libab_tree_refcount_free(libab_tree* tree) {
    if(_tree_needs_free(tree)) {
        libab_tree_free(tree);
        free(tree);
    }
}
