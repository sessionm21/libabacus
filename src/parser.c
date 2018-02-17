#include "parser.h"
#include "util.h"
#include "result.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>

struct parser_state {
    ll_node* current_node;
    libab_lexer_match* current_match;
    const char* string;
    libab_table* base_table;
};

void _parser_state_update(struct parser_state* state) {
    state->current_match = state->current_node ? state->current_node->data : NULL;
}

void _parser_state_init(struct parser_state* state,
        ll* tokens, const char* string, libab_table* table) {
    state->current_node = tokens->head;
    state->string = string;
    state->base_table = table;
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

int _parser_is_type(struct parser_state* state, libab_lexer_token to_expect) {
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

libab_result _parse_block(struct parser_state*, libab_tree**, int);

libab_result _parser_extract_token(struct parser_state* state, char** into, libab_lexer_match* match) {
    libab_result result = LIBAB_SUCCESS;
    size_t string_size = match->to - match->from;
    if((*into = malloc(string_size + 1))) {
        strncpy(*into, state->string + match->from, string_size);
        (*into)[string_size] = '\0';
    } else {
        result = LIBAB_MALLOC;
    }
    return result;
}

int _parser_match_is_op(libab_lexer_match* match) {
    return match->type == TOKEN_OP_INFIX ||
        match->type == TOKEN_OP_PREFIX ||
        match->type == TOKEN_OP_POSTFIX;
}

libab_result _parser_construct_op_node(struct parser_state* state, libab_lexer_match* match, libab_tree** into) {
    libab_result result = LIBAB_SUCCESS;
    if((*into = malloc(sizeof(**into)))) {
        result = _parser_extract_token(state, &(*into)->string_value, match);
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(vec_init(&(*into)->children));
        if(result == LIBAB_SUCCESS) {
            (*into)->variant = OP;
            (*into)->from = match->from;
            (*into)->to = match->to;
            (*into)->line = match->line;
            (*into)->line_from = match->line_from;
        } else {
            free((*into)->string_value);
        }
    } else {
        free(*into);
        *into = NULL;
    }
    return result;
}
libab_result _parser_append_op_node(struct parser_state* state, libab_lexer_match* match, ll* append_to) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* new_tree = NULL;
    result = _parser_construct_op_node(state, match, &new_tree);
    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(ll_append(append_to, new_tree));
        if(result != LIBAB_SUCCESS) {
            libab_tree_free(new_tree);
            free(new_tree);
        }
    } 
    return result;
}
libab_result _parse_expression(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    return result;
}

libab_result _parse_statement(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;

    if(_parser_is_char(state, '{')) result = _parse_block(state, store_into, 1);
    else if(_parser_is_type(state, TOKEN_ID) ||
            _parser_is_type(state, TOKEN_NUM) ||
            _parser_is_char(state, '(') ||
            _parser_is_type(state, TOKEN_OP_PREFIX)) {
        result = _parse_expression(state, store_into);
        if(result == LIBAB_SUCCESS) result = _parser_consume_char(state, ';');
    }

    return result;
}

libab_result _parse_block(struct parser_state* state,
        libab_tree** store_into, int expect_braces) {
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

void libab_parser_init(libab_parser* parser, libab_table* table) {
    parser->base_table = table;
}
libab_result libab_parser_parse(libab_parser* parser, ll* tokens,
        const char* string, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    struct parser_state state;
    _parser_state_init(&state, tokens, string, parser->base_table);

    result = _parse_block(&state, store_into, 0);
    if(result == LIBAB_SUCCESS) {
        (*store_into)->variant = BASE;
    } 

    return result;
}
void libab_parser_free(libab_parser* parser) {
    parser->base_table = NULL;
}
