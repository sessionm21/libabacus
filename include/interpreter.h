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
};

typedef struct libab_interpreter_s libab_interpreter;

/**
 * Initializes an interpreter instance.
 * @param intr the interpreter to initialize.
 * @param ab the libabacus instance this interpreter belongs to.
 */
void libab_interpreter_init(libab_interpreter* intr, struct libab_s* ab);
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
 * Frees the given interpreter.
 * @param intr the interpreter to free.
 */
void libab_interpreter_free(libab_interpreter* intr);

#endif
