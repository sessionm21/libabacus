#ifndef LIBABACUS_PARSER_H
#define LIBABACUS_PARSER_H

#include "table.h"
#include "ll.h"
#include "tree.h"

/**
 * Parses the given list of tokens into the given tree pointer.
 * @param tokens the tokens to use for parsing.
 * @param string the string to use for determining token values.
 * @param store_into tree pointer to store the new data into.
 */
libab_result parse_tokens(ll* tokens, const char* string, tree** store_into);

#endif
