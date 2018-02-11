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
 * A token that is produced by the lexer.
 */
struct lexer_match {
    /**
     * The line that this token was found on.
     */
    size_t line;
    /**
     * The first index at which this token's string
     * begins.
     */
    size_t from;
    /**
     * The index of the first character that is outside
     * this token.
     */
    size_t to;
    /**
     * The index of the beginning of the line on which
     * this token is found.
     */
    size_t line_from;
    /**
     * The type of token.
     */
    int type;
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
typedef struct lexer_match lexer_match;

/**
 * Initializes the given lexer,
 * placing the default tokens into it.
 * @param lexer the lexer to intiailize.
 * @return the result of the operation (can be MALLOC on failed allocation.)
 */
libab_result lexer_init(lexer* lexer);
/**
 * Turns the given input string into tokens.
 * @param lexer the lexer to use to turn the string into tokens.
 * @param string the string to turn into tokens.
 * @param lex_into the list which should be populated with matches.
 * @return the result of the operation.
 */
libab_result lexer_lex(lexer* lexer, const char* string, ll* lext_into);
/**
 * Releases the memory associated with the given lexer,
 * removing all registered patterns from it.
 * @param lexer the lexer to free.
 * @return the result of the operation.
 */
libab_result lexer_free(lexer* lexer);
/**
 * Function intended to be passed to "foreach" calls
 * in libds. lexer_lex allocates matches, and passing this function
 * to foreach will free the memory allocated for the matches.
 */
int lexer_foreach_match_free(void* data, va_list args);

#endif
