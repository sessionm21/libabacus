#include "lexer.h"
#include "ll.h"
#include "util.h"
#include <stdlib.h>
#include <ctype.h>

libab_result libab_lexer_init(libab_lexer* lexer) {
    size_t i;
    libab_result result = LIBAB_SUCCESS;
    const char* words[] = {
        ".",
        "[a-zA-Z][a-zA-Z0-9_]*",
        "[0-9]+(\\.[0-9]*)?",
        "if",
        "else"
    };
    libab_lexer_token tokens[] = {
        TOKEN_CHAR,
        TOKEN_ID,
        TOKEN_NUM,
        TOKEN_KW_IF,
        TOKEN_KW_ELSE,
    };
    const size_t count = sizeof(tokens)/sizeof(libab_lexer_token);

    eval_config_init(&lexer->config);
    for(i = 0; i < count && result == LIBAB_SUCCESS; i++) {
        result = libab_convert_lex_result(
                eval_config_add(&lexer->config, words[i], tokens[i]));
    }

    return result;
}

struct lexer_state {
    size_t line;
    size_t line_from;
    const char* source;
    ll* matches;
};
int _lexer_foreach_convert_match(void* data, va_list args) {
    libab_result result = LIBAB_SUCCESS;
    libab_lexer_match* new_match;
    match* match = data;
    struct lexer_state* state = va_arg(args, struct lexer_state*);
    char first_char = state->source[match->from];
    if(isspace(first_char)) {
        // Skip
    } else if(first_char == '\n') { 
        state->line++;
        state->line_from = match->to;
    } else if((new_match = malloc(sizeof(*new_match)))) {
        new_match->type = match->pattern;
        new_match->from = match->from;
        new_match->to = match->to;
        new_match->line_from = state->line_from;
        new_match->line = state->line;
        result = libab_convert_ds_result(ll_append(state->matches, new_match));
        if(result != LIBAB_SUCCESS) {
            free(new_match);
        }
    } else {
        result = LIBAB_MALLOC;
    }
    return result;
}

libab_result libab_lexer_lex(libab_lexer* lexer, const char* string, ll* lex_into) {
    libab_result result;
    ll raw_matches;
    struct lexer_state state;

    ll_init(&raw_matches);

    state.line = 0;
    state.line_from = 0;
    state.matches = lex_into;
    state.source = string;

    result = libab_convert_lex_result(
            eval_all(string, 0, &lexer->config, &raw_matches));

    if(result == LIBAB_SUCCESS) {
        result = (libab_result) ll_foreach(&raw_matches, NULL, compare_always,
                                           _lexer_foreach_convert_match, &state);
    }

    if(result != LIBAB_SUCCESS) {
        ll_foreach(lex_into, NULL, compare_always, libab_lexer_foreach_match_free);
    }

    ll_foreach(&raw_matches, NULL, compare_always, eval_foreach_match_free);
    ll_free(&raw_matches);

    return result;
}
libab_result libab_lexer_free(libab_lexer* lexer) {
    return libab_convert_lex_result(eval_config_free(&lexer->config));
}
int libab_lexer_foreach_match_free(void* data, va_list args) {
    free((libab_lexer_match*) data);
    return 0;
}
