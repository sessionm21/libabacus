#ifndef LIBABACUS_CUSTOM_H
#define LIBABACUS_CUSTOM_H

#include "parsetype.h"
#include "ref_trie.h"
#include "tree.h"

struct libab_s;

/**
 * A function pointer that is called
 * to execute a certain type of function.
 */
typedef libab_result (*libab_function_ptr)(struct libab_s*, libab_ref_vec*, libab_ref*);

/**
 * The variant of the operator that
 * has been registered with libabacus.
 */
enum libab_operator_variant_e {
    OPERATOR_PREFIX,
    OPERATOR_INFIX,
    OPERATOR_POSTFIX
};

/**
 * The variant of the implementation of a behavior.
 */
enum libab_behavior_variant_e { BIMPL_INTERNAL, BIMPL_TREE };

/**
 * The common information
 * that both operators and functions shared.
 */
struct libab_behavior_s {
    /**
     * The variant of this behavior.
     */
    enum libab_behavior_variant_e variant;
    union {
        /**
         * The internal function used for an internal implementation.
         */
        libab_function_ptr internal;
        /**
         * The tree-based implementation.
         */
        libab_tree* tree;
    } data_u;
};

/**
 * A struct that holds informatiion
 * about an operator that has been
 * registered with libabacus.
 */
struct libab_operator_s {
    /**
     * The type of the operator (infix, prefix, postfix).
     * Corresponds to token types associated with
     * each operator.
     */
    enum libab_operator_variant_e variant;
    /**
     * The precedence of the operator.
     */
    int precedence;
    /**
     * The associativity of the operator.
     */
    int associativity;
    /**
     * The function called by this operator.
     */
    const char* function;
};

/**
 * A struct that holds information
 * about an function that has been
 * registered with libabacus.
 */
struct libab_function_s {
    /**
     * The behavior of this function.
     */
    struct libab_behavior_s behavior;
    /**
     * The parameters given to this function
     * if it was created via partial application.
     */
    libab_ref_vec params;
    /**
     * The scope in which this function was declared.
     */
    libab_ref scope;
};

typedef enum libab_operator_variant_e libab_operator_variant;
typedef enum libab_behavior_variant_e libab_behavior_variant;
typedef struct libab_behavior_s libab_behavior;
typedef struct libab_operator_s libab_operator;
typedef struct libab_function_s libab_function;

/**
 * Initializes a behavior that uses an internal function.
 * @param behavior the behavior to initialize.
 * @param func the function that this behavior calls.
 */
void libab_behavior_init_internal(libab_behavior* behavior,
                                  libab_function_ptr func);
/**
 * Initializes a behavior that uses a tree that has been
 * parsed from the user.
 * @param behavior the behavior to initialize.
 * @param tree the tree that this behavior uses.
 */
void libab_behavior_init_tree(libab_behavior* behavior, libab_tree* tree);
/**
 * Frees the given behavior.
 * @param behavior the behavior to free.
 */
void libab_behavior_free(libab_behavior* behavior);
/**
 * Initializes an operator with the given info.
 * @param op the operator to initialize.
 * @param variant the variant of the operator (infix, prefix, etc)
 * @param precedence the precedence of the operator.
 * @param associativity the associativity (left = -1, right = 1) of the
 * operator.
 * @param function the function this operator represents.
 */
void libab_operator_init(libab_operator* op, libab_operator_variant variant,
                         int precedence, int associativity, const char* function);
/**
 * Frees the given operator.
 * @param op the operator to free.
 */
void libab_operator_free(libab_operator* op);
/**
 * Initializes a function with the given internal behavior.
 * @param function the function to initialize.
 * @param fun the function implementation.
 * @param the parent scope in which this function was declared.
 * @return the result of the initialization.
 */
libab_result libab_function_init_internal(libab_function* function,
                                          libab_function_ptr fun,
                                          libab_ref* scope);
/**
 * Initializes a function with the given tree behavior.
 * @param function the function to initialize.
 * @param tree the tree that represents the function's behavior.
 * @param scope the scope in which this function was declared.
 * @return the result of the initialization.
 */
libab_result libab_function_init_tree(libab_function* function,
                                      libab_tree* tree,
                                      libab_ref* scope);
/**
 * Frees the given function.
 * @param fun the function to free.
 */
void libab_function_free(libab_function* fun);

#endif
