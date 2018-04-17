#ifndef LIBABACUS_CUSTOM_H
#define LIBABACUS_CUSTOM_H

#include "parsetype.h"

/**
 * A function pointer that is called
 * to execute a certain type of function.
 */
typedef void(*libab_function_ptr)();

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
 * The common information
 * that both operators and functions shared.
 */
struct libab_behavior_s {
    /**
     * The function that handles the parameters.
     */
    libab_function_ptr function;
    /**
     * The type of the function.
     */
    libab_parsetype* type;
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
    enum libab_operator_variant_e type;
    /**
     * The precedence of the operator.
     */
    int precedence;
    /**
     * The associativity of the operator.
     */
    int associativity;
    /**
     * The behavior of this operator.
     */
    struct libab_behavior_s behavior;
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
};

typedef enum libab_operator_variant_e libab_operator_variant;
typedef struct libab_behavior_s libab_behavior;
typedef struct libab_operator_s libab_operator;
typedef struct libab_function_s libab_function;

/**
 * Frees the given behavior.
 * @param behavior the behavior to free.
 */
void libab_behavior_free(libab_behavior* behavior);
/**
 * Frees the given operator.
 * @param op the operator to free.
 */
void libab_operator_free(libab_operator* op);
/**
 * Frees the given function.
 * @param fun the function to free.
 */
void libab_function_free(libab_function* fun);

#endif
