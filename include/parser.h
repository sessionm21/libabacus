#ifndef LIBABACUS_PARSER_H
#define LIBABACUS_PARSER_H

#include "table.h"
#include "ll.h"
#include "tree.h"

/**
 * The parser that is used by libabacus
 * to store information for converting
 * tokens into trees.
 */
struct libab_parser_s {
    libab_table* base_table;
};

typedef struct libab_parser_s libab_parser;

/**
 * Initializes the parser.
 * @param parser the parser to intialize.
 * @param table the table of "reserved" entries like operators.
 */
void libab_parser_init(libab_parser* parser, libab_table* table);
/**
 * Parses the given list of tokens into the given tree pointer.
 * @param parser the parser to use for parsing text.
 * @param tokens the tokens to use for parsing.
 * @param string the string to use for determining token values.
 * @param store_into tree pointer to store the new data into.
 */
libab_result libab_parser_parse(libab_parser* parser, ll* tokens,
        const char* string, libab_tree** store_into);
/**
 * Releases the resources allocated by the parser.
 * @param parser the parser to release.
 */
void libab_parser_free(libab_parser* parser);

#endif
