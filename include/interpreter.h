#ifndef LIBABACUS_INTERPRETER_H
#define LIBABACUS_INTERPRETER_H

#include "table.h"
#include "tree.h"
#include "impl.h"

struct libab_interpreter_s {
    libab_ref base_table;
    libab_impl* impl;
};

typedef struct libab_interpreter_s libab_interpreter;

void libab_interpreter_init(libab_interpreter* intr,
                            libab_ref* table,
                            libab_impl* impl);
libab_result libab_interpreter_run(libab_interpreter* intr,
                                   libab_tree* tree, libab_ref* into);
void libab_interpreter_free(libab_interpreter* intr);

#endif
