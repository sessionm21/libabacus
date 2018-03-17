#include "debug.h"

int _debug_foreach_print_tree(void* data, va_list args);

const char* _debug_node_name(libab_tree_variant var) {
    static const char* names[] = {
        "none",
        "base",
        "id",
        "num",
        "op",
        "reserved_op",
        "unary_op",
        "block",
        "void",
        "if",
        "while",
        "dowhile",
        "call",
        "fun",
        "fun_param",
        "return"
    };
    return names[var];
}

void _debug_print_tree_node(libab_tree* tree, FILE* file) {
    fprintf(file, "%s", _debug_node_name(tree->variant));
    if(libab_tree_has_string(tree->variant)) {
        fprintf(file, ": %s", tree->string_value);
    }
    fprintf(file, "\n");
}

void _debug_print_tree(libab_tree* tree, FILE* file, int depth) {
    int i = depth;
    while(i--) printf("  ");
    _debug_print_tree_node(tree, file);
    if(libab_tree_has_vector(tree->variant)) {
        vec_foreach(&tree->children, NULL, compare_always,
                _debug_foreach_print_tree, file, depth + 1);
    }
}

int _debug_foreach_print_tree(void* data, va_list args) {
    FILE* file = va_arg(args, FILE*);
    int depth = va_arg(args, int);
    _debug_print_tree(data, file, depth);
    return 0;
}

void libab_debug_fprint_tree(libab_tree* print, FILE* file) {
    _debug_print_tree(print, file, 0);
}

void libab_debug_print_tree(libab_tree* print) {
    _debug_print_tree(print, stdout, 0);
}
