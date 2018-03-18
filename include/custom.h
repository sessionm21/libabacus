#ifndef LIBABACUS_CUSTOM_H
#define LIBABACUS_CUSTOM_H

#include "parsetype.h"

/**
 * A function pointer that is called
 * to execute a certain type of function.
 */
typedef void(*libab_function_ptr)();

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
    int type;
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

typedef struct libab_behavior_s libab_behavior;
typedef struct libab_operator_s libab_operator;
typedef struct libab_function_s libab_function;

#endif
