#include "libabacus.h"
#include "parser.h"
#include "libabacus_util.h"
#include "lexer.h"
#include <stdlib.h>

struct parser_state {
    ll_node* current_node;
    lexer_match* current_match;
    const char* string;
};

void _parser_state_update(struct parser_state* state) {
    state->current_match = state->current_node ? state->current_node->data : NULL;
}

void _parser_state_init(struct parser_state* state, ll* tokens, const char* string) {
    state->current_node = tokens->head;
    state->string = string;
    _parser_state_update(state);
}

void _parser_state_step(struct parser_state* state) {
    if(state->current_node) {
        state->current_node = state->current_node->next;
    }
    _parser_state_update(state);
}

int _parser_is_char(struct parser_state* state, char to_expect) {
    return (state->current_match && state->current_match->type == TOKEN_CHAR &&
            state->string[state->current_match->from] == to_expect);
}

int _parser_is_type(struct parser_state* state, lexer_token to_expect) {
    return (state->current_match && state->current_match->type == to_expect);
}

int _parser_eof(struct parser_state* state) {
    return state->current_match == NULL;
}

libab_result _parser_consume_char(struct parser_state* state, char to_consume) {
    libab_result result = LIBAB_SUCCESS;
    if(state->current_match == NULL) {
        result = LIBAB_EOF;
    } else if(state->current_match->type != TOKEN_CHAR ||
            state->string[state->current_match->from] != to_consume) {
        result = LIBAB_UNEXPECTED;
    } else {
        _parser_state_step(state);
    }
    return result;
}

libab_result _parse_block(struct parser_state*, tree**, int);

libab_result _parse_expression(struct parser_state* state, tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    return result;
}

libab_result _parse_statement(struct parser_state* state, tree** store_into) {
    libab_result result = LIBAB_SUCCESS;

    if(_parser_is_char(state, '{')) result = _parse_block(state, store_into, 1);
    else if(_parser_is_type(state, TOKEN_ID) ||
            _parser_is_type(state, TOKEN_NUM) ||
            _parser_is_type(state, TOKEN_STR) ||
            _parser_is_type(state, TOKEN_CHAR_LIT) ||
            _parser_is_type(state, TOKEN_TRUE) ||
            _parser_is_type(state, TOKEN_FALSE) ||
            _parser_is_char(state, '(') ||
            _parser_is_type(state, TOKEN_OP_PREFIX)) {
        result = _parse_expression(state, store_into);
        if(result == LIBAB_SUCCESS) result = _parser_consume_char(state, ';');
    }

    return result;
}

libab_result _parse_block(struct parser_state* state,
        tree** store_into, int expect_braces) {
    libab_result result = LIBAB_SUCCESS;
    if((*store_into = malloc(sizeof(**store_into))) == NULL) result = LIBAB_MALLOC;

    if(expect_braces && result == LIBAB_SUCCESS) result = _parser_consume_char(state, '{');

    while(result == LIBAB_SUCCESS && 
            !_parser_eof(state) &&
            !(expect_braces && _parser_is_char(state, '}'))) {
        
    }

    if(expect_braces && result == LIBAB_SUCCESS) result = _parser_consume_char(state, '}');
    return result;
}

libab_result parse_tokens(ll* tokens, const char* string, tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    struct parser_state state;
    _parser_state_init(&state, tokens, string);

    result = _parse_block(&state, store_into, 0);
    if(result == LIBAB_SUCCESS) {
        (*store_into)->variant = BASE;
    } 

    return result;
}
