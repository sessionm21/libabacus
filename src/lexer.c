#include "lexer.h"
#include "libabacus_util.h"

libab_result lexer_init(lexer* lexer) {
    size_t i;
    libab_result result = LIBAB_SUCCESS;
    const char* words[] = {
        ".",
        "[a-zA-Z][a-zA-Z0-9_]*",
        "true",
        "false",
        "[0-9]+(\\.[0-9]*)",
        "\"[^\"]*\"",
        "'[^']'",
        "fun",
        "if",
        "else",
        "while",
        "do",
        "for",
        "return"
    };
    lexer_token tokens[] = {
        TOKEN_CHAR,
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
        TOKEN_KW_RETURN
    };
    const size_t count = sizeof(tokens)/sizeof(lexer_token);

    eval_config_init(&lexer->config);
    for(i = 0; i < count && result == LIBAB_SUCCESS; i++) {
        result = convert_lex_result(
                eval_config_add(&lexer->config, words[i], tokens[i]));
    }

    return result;
}
libab_result lexer_free(lexer* lexer) {
    return convert_lex_result(eval_config_free(&lexer->config));
}
