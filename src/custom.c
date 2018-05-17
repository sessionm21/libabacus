#include "custom.h"

void libab_behavior_init_internal(libab_behavior* behavior,
                                  libab_function_ptr func) {
    behavior->variant = BIMPL_INTERNAL;
    behavior->data_u.internal = func;
}

void libab_behavior_init_tree(libab_behavior* behavior,
                              libab_tree* tree) {
    behavior->variant = BIMPL_TREE;
    behavior->data_u.tree = tree;
}

void libab_behavior_free(libab_behavior* behavior) {
    if (behavior->variant == BIMPL_TREE) {
        libab_tree_free_recursive(behavior->data_u.tree);
    }
}

void libab_operator_init(libab_operator* op, libab_operator_variant variant, 
                         int precedence, int associativity, libab_ref* type,
                        libab_function_ptr func) {
    op->variant = variant;
    op->precedence = precedence;
    op->associativity = associativity;
    libab_ref_copy(type, &op->type);
    libab_behavior_init_internal(&op->behavior, func);
}

void libab_operator_free(libab_operator* op) {
    libab_ref_free(&op->type);
    libab_behavior_free(&op->behavior);
}

libab_result _function_init(libab_function* function) {
    return LIBAB_SUCCESS;
}
libab_result libab_function_init_internal(libab_function* function,
                                          libab_function_ptr fun) {
    libab_result result = _function_init(function);
    libab_behavior_init_internal(&function->behavior, fun);
    return result;
}
libab_result libab_function_init_tree(libab_function* function,
                                      libab_tree* tree) {
    libab_result result = _function_init(function);
    libab_behavior_init_tree(&function->behavior, tree);
    return result;
}
void libab_function_free(libab_function* fun) {
    libab_behavior_free(&fun->behavior);
}
