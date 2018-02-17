#include "tree.h"
#include <stdlib.h>

void libab_tree_free(libab_tree* tree) {
    int free_string = 0;
    int free_vector = 0;
    if(tree->variant == BASE || tree->variant == OP ||
            tree->variant == UNARY_OP || tree->variant == BLOCK ||
            tree->variant == FUN || tree->variant == IF ||
            tree->variant == WHILE || tree->variant == DOWHILE ||
            tree->variant == FOR || tree->variant == CALL ||
            tree->variant == RETURN) {
        free_vector = 1;
    }
    if(tree->variant == ID || tree->variant == STR || tree->variant == NUM ||
            tree->variant == KW || tree->variant == OP || tree->variant == UNARY_OP ||
            tree->variant == FUN || tree->variant == CALL) {
        free_string = 1;
    }
    if(free_string) free(tree->string_value);
    if(free_vector) vec_free(&tree->children);
}
