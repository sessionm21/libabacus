#ifndef LIBABACUS_INTERPRETER_H
#define LIBABACUS_INTERPRETER_H

#include "impl.h"
#include "libabacus.h"
#include "table.h"
#include "tree.h"

struct libab_s;

/**
 * Scope moe used to determine how the interpreter handles
 * scoping.
 */
enum libab_interpreter_scope_mode_e {
    SCOPE_NORMAL,
    SCOPE_FORCE,
    SCOPE_NONE
};

/**
 * The interpreter struct used to encapsulate
 * any interpreter-specific data.
 */
struct libab_interpreter_s {
    struct libab_s* ab;
    /**
     * The unit value, which doesn't need more than one instance.
     */
    libab_ref value_unit;
    /**
     * The "true" boolean value, which doesn't need more than one instance.
     */
    libab_ref value_true;
    /**
     * The "false" boolean value, which doesn't need more than one instance.
     */
    libab_ref value_false;
};

typedef enum libab_interpreter_scope_mode_e libab_interpreter_scope_mode;
typedef struct libab_interpreter_s libab_interpreter;

/**
 * Initializes an interpreter instance.
 * @param intr the interpreter to initialize.
 * @param ab the libabacus instance this interpreter belongs to.
 */
libab_result libab_interpreter_init(libab_interpreter* intr, struct libab_s* ab);
/**
 * Uses the interpreter to run the given parse tree.
 * @param intr the interpreter to use to run the code.
 * @param tree the tree to run.
 * @param scope the parent scope to use for running the tree.
 * @param mode the scope mode to use.
 * @param into the reference into which the result of the execution will be
 * stored.
 * @return the result of the execution.
 */
libab_result libab_interpreter_run(libab_interpreter* intr, libab_tree* tree,
                                   libab_ref* scope,
                                   libab_interpreter_scope_mode mode,
                                   libab_ref* into);
/**
 * Calls a function with the given parameters.
 * @param intr the interpreter to use to call the function.
 * @param scope the scope in which the function should be searched for.
 * @param function the function to call.
 * @param params the parameters to pass to the function.
 * @param into the reference to store the result into.
 * @return the result of the call.
 */
libab_result libab_interpreter_call_function(libab_interpreter* intr,
                                            libab_ref* scope,
                                            const char* function,
                                            libab_ref_vec* params,
                                            libab_ref* into);
/**
 * Calls a function value with the given parameters.
 * @param intr the interpreter to use to call the function.
 * @param scope the scope in which the function should be searched for.
 * @param function the function to call.
 * @param params the parameters to pass to the function.
 * @param into the reference to store the result into.
 * @return the result of the call.
 */
libab_result libab_interpreter_call_value(libab_interpreter* intr,
                                         libab_ref* scope,
                                         libab_ref* function,
                                         libab_ref_vec* params,
                                         libab_ref* into);
/**
 * Gets the unit value from this interpreter.
 * @param intr the interpreter from which to get the unit value.
 * @param into the reference into which to store the unit value.
 */
void libab_interpreter_unit_value(libab_interpreter* intr, libab_ref* into);
/**
 * Gets the true value from this interpreter.
 * @param intr the interpreter from which to get the true value.
 * @param into the reference into which to store the true value.
 */
void libab_interpreter_true_value(libab_interpreter* intr, libab_ref* into);
/**
 * Gets the false value from this interpreter.
 * @param intr the interpreter from which to get the false value.
 * @param into the reference into which to store the false value.
 */
void libab_interpreter_false_value(libab_interpreter* intr, libab_ref* into);
/**
 * Frees the given interpreter.
 * @param intr the interpreter to free.
 */
void libab_interpreter_free(libab_interpreter* intr);

#endif
