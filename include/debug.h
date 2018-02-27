#ifndef LIBABACUS_DEBUG_H
#define LIBABACUS_DEBUG_H

#include "tree.h"
#include <stdio.h>

void libab_debug_fprint_tree(libab_tree* print, FILE* file);
void libab_debug_print_tree(libab_tree* print);

#endif
