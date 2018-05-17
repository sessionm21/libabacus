#ifndef LIBABACUS_INTERPRETER_H
#define LIBABACUS_INTERPRETER_H

#include "impl.h"
#include "libabacus.h"
#include "table.h"
#include "tree.h"

struct libab_s;

struct libab_interpreter_s {
    struct libab_s* ab;
};

typedef struct libab_interpreter_s libab_interpreter;

void libab_interpreter_init(libab_interpreter* intr, struct libab_s* ab);
libab_result libab_interpreter_run(libab_interpreter* intr, libab_tree* tree,
                                   libab_ref* into);
void libab_interpreter_free(libab_interpreter* intr);

#endif
