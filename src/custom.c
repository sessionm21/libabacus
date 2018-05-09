#include "custom.h"

void libab_behavior_init_internal(libab_behavior* behavior, libab_ref* type,
                                  libab_function_ptr func) {
    behavior->impl.variant = BIMPL_INTERNAL;
    behavior->impl.data_u.internal = func;
    libab_ref_copy(type, &behavior->type);
}

void libab_behavior_init_tree(libab_behavior* behavior, libab_ref* type,
                              libab_tree* tree) {
    behavior->impl.variant = BIMPL_TREE;
    behavior->impl.data_u.tree = tree;
    libab_ref_copy(type, &behavior->type);
}

void libab_behavior_free(libab_behavior* behavior) {
    libab_ref_free(&behavior->type);
    if (behavior->impl.variant == BIMPL_TREE) {
        libab_tree_free_recursive(behavior->impl.data_u.tree);
    }
}

void libab_operator_init(libab_operator* op, libab_operator_variant variant, 
                         int precedence, int associativity, libab_ref* type,
                        libab_function_ptr func) {
    op->type = variant;
    op->precedence = precedence;
    op->associativity = associativity;
    libab_behavior_init_internal(&op->behavior, type, func);
}

void libab_operator_free(libab_operator* op) {
    libab_behavior_free(&op->behavior);
}

libab_result _function_init(libab_function* function) {
    return LIBAB_SUCCESS;
}
libab_result libab_function_init_internal(libab_function* function, libab_ref* type,
                                          libab_function_ptr fun) {
    libab_result result = _function_init(function);
    libab_behavior_init_internal(&function->behavior, type, fun);
    return result;
}
libab_result libab_function_init_tree(libab_function* function, libab_ref* type,
                                      libab_tree* tree) {
    libab_result result = _function_init(function);
    libab_behavior_init_tree(&function->behavior, type, tree);
    return result;
}
void libab_function_free(libab_function* fun) {
    libab_behavior_free(&fun->behavior);
}
