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

/**
 * The various tokens used by the lexer
 * in order to tag meaningful sequences
 * of characters.
 */
enum lexer_token {
    TOKEN_CHAR = 0,
    TOKEN_ID,
    TOKEN_TRUE,
    TOKEN_FALSE,
    TOKEN_NUM,
    TOKEN_STR,
    TOKEN_CHAR_LIT,
    TOKEN_KW_FUN,
    TOKEN_KW_IF,
    TOKEN_KW_ELSE,
    TOKEN_KW_WHILE,
    TOKEN_KW_DO,
    TOKEN_KW_FOR,
    TOKEN_KW_RETURN,
    TOKEN_OP_INFIX,
    TOKEN_OP_POSTFIX,
    TOKEN_OP_PREFIX,
    TOKEN_LAST
};

typedef struct lexer lexer;
typedef enum lexer_token lexer_token;

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
