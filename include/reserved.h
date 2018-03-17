#ifndef LIBABACUS_RESERVED_H
#define LIBABACUS_RESERVED_H

#include "result.h"
#include "parsetype.h"
#include "tree.h"
#include "value.h"
#include "libabacus.h"

/**
 * Struct that represents a reserved operator that contains
 * interpreter-internal behavior.
 */
struct libab_reserved_operator_s {
    /**
     * The reserved operator.
     */
    const char* op;
    /**
     * The precedence of this operator.
     */
    int precedence;
    /**
     * The associativity of this operator.
     */
    int associativity;
};

typedef struct libab_reserved_operator_s libab_reserved_operator;

/**
 * Attempts to find a reserved operator with the given name.
 * This function operates under the assumption that there are
 * few reserved operators in libabacus. As such, it does effectively
 * a polynomial time search - strcmp for every element until the operator is found.
 * @param name the name to search for.
 * @return the reserved operator, if it is found.
 */
const libab_reserved_operator* libab_find_reserved_operator(const char* name);
/**
 * Registers the existing reserved operators into the given lexer.
 * @param lexer the lexer to register into.
 * @return the result of the registration.
 */
libab_result libab_register_reserved_operators(libab_lexer* lexer);
/**
 * Remove the existing reserved operators from the given lexer.
 * @param lexer the lexer to remove from.
 * @return the result of the removal.
 */
libab_result libab_remove_reserved_operators(libab_lexer* lexer);

#endif
