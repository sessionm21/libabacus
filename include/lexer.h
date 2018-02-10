#ifndef LIBABACUS_LEXER_H
#define LIBABACUS_LEXER_H

#include "eval.h"
#include "libabacus.h"

/**
 * The lexer used for reading
 * a string and converting it into
 * tokens.
 */
struct lexer {
    /**
     * The liblex configuration used
     * to convert the string into tokens.
     */
    eval_config config;
};

typedef struct lexer lexer;

/**
 * Initializes the given lexer,
 * placing the default tokens into it.
 * @param lexer the lexer to intiailize.
 * @return the result of the operation (can be MALLOC on failed allocation.)
 */
libab_result lexer_init(lexer* lexer);
/**
 * Releases the memory associated with the given lexer,
 * removing all registered patterns from it.
 * @param lexer the lexer to free.
 * @return the result of the operation.
 */
libab_result lexer_free(lexer* lexer);

#endif
