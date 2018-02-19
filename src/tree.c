#include "tree.h"
#include <stdlib.h>

int _tree_has_vector(libab_tree* tree) {
    return tree->variant == BASE || tree->variant == OP ||
            tree->variant == UNARY_OP || tree->variant == BLOCK ||
            tree->variant == IF;
}

void libab_tree_free(libab_tree* tree) {
    int free_string = 0;
    int free_vector = 0;
    if(_tree_has_vector(tree)) {
        free_vector = 1;
    }
    if(tree->variant == ID || tree->variant == NUM ||
            tree->variant == OP || tree->variant == UNARY_OP) {
        free_string = 1;
    }
    if(free_string) free(tree->string_value);
    if(free_vector) vec_free(&tree->children);
}

int _tree_foreach_free(void* data, va_list args) {
    libab_tree_free_recursive(data);
    return 0;
}

void libab_tree_free_recursive(libab_tree* tree) {
    if(_tree_has_vector(tree)) {
        vec_foreach(&tree->children, NULL, compare_always, _tree_foreach_free);
    }
    libab_tree_free(tree);
    free(tree);
}
