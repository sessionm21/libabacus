#ifndef LIBABACUS_INTERPRETER_H
#define LIBABACUS_INTERPRETER_H

#include "impl.h"
#include "libabacus.h"
#include "table.h"
#include "tree.h"

struct libab_s;

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
};

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
 * @param into the reference into which the result of the execution will be
 * stored.
 * @return the result of the execution.
 */
libab_result libab_interpreter_run(libab_interpreter* intr, libab_tree* tree,
                                   libab_ref* into);
/**
 * Calls a function with the given parameters.
 * @param intr the interpreter to use to call the function.
 * @param function the function to call.
 * @param params the parameters to pass to the function.
 * @param into the reference to store the result into.
 * @return the result of the call.
 */
libab_result libab_interpreter_run_function(libab_interpreter* intr,
                                            const char* function,
                                            libab_ref_vec* params,
                                            libab_ref* into);
/**
 * Gets the unit value from this interpreter.
 * @param intr the interpreter from which to get the unit value.
 * @param into the reference into which to store the unit value.
 */
void libab_interpreter_unit_value(libab_interpreter* intr, libab_ref* into);
/**
 * Frees the given interpreter.
 * @param intr the interpreter to free.
 */
void libab_interpreter_free(libab_interpreter* intr);

#endif
