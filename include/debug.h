#ifndef LIBABACUS_DEBUG_H
#define LIBABACUS_DEBUG_H

#include "tree.h"
#include <stdio.h>

/**
 * Prints the given tree to the given file.
 * @param print the tree to print.
 * @param file the file to print to.
 */
void libab_debug_fprint_tree(libab_tree* print, FILE* file);
/**
 * Prints the given tree to the standard output.
 * @param print the tree to print.
 */
void libab_debug_print_tree(libab_tree* print);

#endif
