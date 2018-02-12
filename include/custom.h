#ifndef LIBABACUS_CUSTOM_H
#define LIBABACUS_CUSTOM_H

/**
 * A function pointer that is called
 * to execute a certain type of function.
 */
typedef void(*libab_function_ptr)();

/**
 * A struct that holds informatiion
 * about an operator that has been
 * registered with libabacus.
 */
struct libab_operator_s {
    /**
     * The precedence of the operator.
     */
    int precedence;
    /**
     * The functionality of the operator.
     */
    libab_function_ptr function;
};

/**
 * A struct that holds information
 * about an function that has been
 * registered with libabacus.
 */
struct libab_function_s {
    /**
     * The functionality of the function.
     */
    libab_function_ptr function;
};

typedef struct libab_operator_s libab_operator;
typedef struct libab_function_s libab_function;

#endif
