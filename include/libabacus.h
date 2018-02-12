#ifndef LIBABACUS_H
#define LIBABACUS_H

#include "ht.h"
#include "lexer.h"
#include "table.h"
#include "parser.h"
#include "result.h"

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


/**
 * The main struct of libabacus,
 * which essentially holds all the informatiom
 * for the library's state and operation.
 */
struct libab_s {
    /**
     * The lexer used to convert a string
     * to tokens.
     */
    libab_lexer lexer;
    /**
     * The parser used to convert
     * tokens to a tree.
     */
    libab_parser parser;
    /**
     * The table used to store top-level
     * things like functions and operators.
     */
    libab_table table;
};

typedef struct libab_operator_s libab_operator;
typedef struct libab_function_s libab_function;
typedef struct libab_s libab;

/**
 * Initializes the libabacus struct as well
 * as all its internal structures such as the lexer.
 * @param ab the libabacus instance used to keep state.
 * @return the result of the initialization.
 */
libab_result libab_init(libab* ab);
/**
 * Registers an operator with libabacus.
 * @param ab the libabacus instance to reigster the operator with.
 * @param op the operator string to register.
 * @param precedence the precedence of the operator.
 * @param func the function that describes the functionality of the operator.
 * @return the result of the initialization.
 */
libab_result libab_register_operator(libab* ab, const char* op, int precedence, libab_function_ptr func);
/**
 * Registers a function with libabacus.
 * @param ab the libabacus instance used to keep state.
 * @param name the name of the function.
 * @param func the function that describes the functionality of the function.
 * @return the result of the initialization.
 */
libab_result libab_register_function(libab* ab, const char* name, libab_function_ptr func);
/**
 * Releases all the resources allocated by libabacus.
 * @param ab the libabacus instance to release.
 * @return the result of the initialization.
 */
libab_result libab_free(libab* ab);

#endif
