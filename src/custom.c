#include "custom.h"
#include "util.h"

void libab_behavior_init_internal(libab_behavior* behavior,
                                  libab_function_ptr func) {
    behavior->variant = BIMPL_INTERNAL;
    behavior->data_u.internal = func;
}

void libab_behavior_init_tree(libab_behavior* behavior, libab_tree* tree) {
    behavior->variant = BIMPL_TREE;
    behavior->data_u.tree = tree;
    tree->int_value++;
}

void libab_behavior_copy(libab_behavior* behavior, libab_behavior* into) {
    into->variant = behavior->variant;
    into->data_u = behavior->data_u;
    if(into->variant == BIMPL_TREE) {
        into->data_u.tree->int_value++;
    }
}

void libab_behavior_free(libab_behavior* behavior) {
    if (behavior->variant == BIMPL_TREE) {
        libab_tree_free_recursive(behavior->data_u.tree);
    }
}

libab_result libab_operator_init(libab_operator* op, libab_operator_variant variant,
                         int precedence, int associativity, const char* function) {
    libab_result result = LIBAB_SUCCESS;
    char* into;
    op->variant = variant;
    op->precedence = precedence;
    op->associativity = associativity;
    result = libab_copy_string(&into, function);
    op->function = into;
    return result;
}

void libab_operator_free(libab_operator* op) {
    free((char*) op->function);
}

libab_result _function_init(libab_function* function, libab_ref* scope) {
    libab_ref_copy(scope, &function->scope);
    return libab_ref_vec_init(&function->params);
}
libab_result libab_function_init_internal(libab_function* function,
                                          libab_function_ptr fun,
                                          libab_ref* scope) {
    libab_result result = _function_init(function, scope);
    if(result == LIBAB_SUCCESS)
        libab_behavior_init_internal(&function->behavior, fun);
    return result;
}
libab_result libab_function_init_tree(libab_function* function,
                                      libab_tree* tree,
                                      libab_ref* scope) {
    libab_result result = _function_init(function, scope);
    if(result == LIBAB_SUCCESS)
        libab_behavior_init_tree(&function->behavior, tree);
    return result;
}
libab_result libab_function_init_behavior(libab_function* function,
                                          libab_behavior* behavior,
                                          libab_ref* scope) {
    libab_result result = _function_init(function, scope);
    if(result == LIBAB_SUCCESS)
        libab_behavior_copy(behavior, &function->behavior);
    return result;
}
void libab_function_free(libab_function* fun) {
    libab_behavior_free(&fun->behavior);
    libab_ref_vec_free(&fun->params);
    libab_ref_free(&fun->scope);
}
