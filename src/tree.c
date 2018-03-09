#include "tree.h"
#include <stdlib.h>

int libab_tree_has_vector(libab_tree_variant variant) {
    return variant == BASE || variant == OP ||
            variant == UNARY_OP || variant == BLOCK ||
            variant == IF || variant == CALL || variant == WHILE ||
            variant == DOWHILE || variant == FUN;
}

int libab_tree_has_string(libab_tree_variant variant) {
    return variant == ID || variant == NUM ||
            variant == OP || variant == UNARY_OP ||
            variant == FUN || variant == FUN_PARAM;
}

int libab_tree_has_type(libab_tree_variant variant) {
    return variant == FUN_PARAM || variant == FUN;
}

void libab_tree_free(libab_tree* tree) {
    int free_string = 0;
    int free_vector = 0;
    int free_type = 0;
    free_vector = libab_tree_has_vector(tree->variant);
    free_string = libab_tree_has_string(tree->variant);
    free_type = libab_tree_has_type(tree->variant);
    if(free_string) free(tree->string_value);
    if(free_vector) vec_free(&tree->children);
    if(free_type && tree->parse_type)
        libab_parsetype_free_recursive(tree->parse_type);
}

int _tree_foreach_free(void* data, va_list args) {
    libab_tree_free_recursive(data);
    return 0;
}

void libab_tree_free_recursive(libab_tree* tree) {
    if(libab_tree_has_vector(tree->variant)) {
        vec_foreach(&tree->children, NULL, compare_always, _tree_foreach_free);
    }
    libab_tree_free(tree);
    free(tree);
}
