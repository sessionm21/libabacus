#include "debug.h"
#include <stdio.h>

int _debug_foreach_print_tree(void* data, va_list args);

const char* _debug_node_name(libab_tree_variant var) {
    static const char* names[] = {
        "none",
        "base",
        "id",
        "num",
        "op",
        "unary_op",
        "block",
        "void",
        "if",
        "call"
    };
    return names[var];
}

void _debug_print_tree_node(libab_tree* tree) {
    printf("%s", _debug_node_name(tree->variant));
    if(libab_tree_has_string(tree->variant)) {
        printf(": %s", tree->string_value);
    }
    printf("\n");
}

void _debug_print_tree(libab_tree* tree, int depth) {
    int i = depth;
    while(i--) printf("  ");
    _debug_print_tree_node(tree);
    if(libab_tree_has_vector(tree->variant)) {
        vec_foreach(&tree->children, NULL, compare_always,
                _debug_foreach_print_tree, depth + 1);
    }
}

int _debug_foreach_print_tree(void* data, va_list args) {
    int depth = va_arg(args, int);
    _debug_print_tree(data, depth);
    return 0;
}

void libab_debug_print_tree(libab_tree* print) {
    _debug_print_tree(print, 0);
}
