#ifndef LIBABACUS_PARSER_H
#define LIBABACUS_PARSER_H

#include "ll.h"
#include "parsetype.h"
#include "table.h"
#include "tree.h"

struct libab_s;

/**
 * The parser that is used by libabacus
 * to store information for converting
 * tokens into trees.
 */
struct libab_parser_s {
    struct libab_s* ab;
};

typedef struct libab_parser_s libab_parser;

/**
 * Initializes the parser.
 * @param parser the parser to intialize.
 * @param table the table of "reserved" entries like operators.
 */
void libab_parser_init(libab_parser* parser, struct libab_s* ab);
/**
 * Parses the given list of tokens into the given tree pointer.
 * @param parser the parser to use for parsing text.
 * @param tokens the tokens to use for parsing.
 * @param string the string to use for determining token values.
 * @param store_into tree pointer to store the new data into.
 * @return the result of parsing the tree.
 */
libab_result libab_parser_parse(libab_parser* parser, ll* tokens,
                                const char* string, libab_tree** store_into);
/**
 * Parses a type into the given reference.
 * @param parser the parser to use for parsing text.
 * @param tokens the tokens to use for parsing.
 * @param string the string from which the tokens came from.
 * @param store_into the reference into which to place the type.
 * @return the result of parsing the type.
 */
libab_result libab_parser_parse_type(libab_parser* parser, ll* tokens,
                                     const char* string, libab_ref* store_into);
/**
 * Releases the resources allocated by the parser.
 * @param parser the parser to release.
 */
void libab_parser_free(libab_parser* parser);

#endif
