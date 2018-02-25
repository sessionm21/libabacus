#include "parser.h"
#include "util.h"
#include "result.h"
#include "lexer.h"
#include <stdlib.h>
#include <string.h>

struct parser_state {
    ll_node* current_node;
    libab_lexer_match* current_match;
    libab_lexer_match* last_match;
    const char* string;
    libab_table* base_table;
};

// Utilities
int _parser_foreach_free_tree(void* data, va_list args) {
    libab_tree_free(data);
    free(data);
    return 0;
}

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

// State functions
void _parser_state_update(struct parser_state* state) {
    state->current_match = state->current_node ? state->current_node->data : NULL;
    if(state->current_match) state->last_match = state->current_match;
}

void _parser_state_init(struct parser_state* state,
        ll* tokens, const char* string, libab_table* table) {
    state->last_match = NULL;
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

libab_result _parser_consume_type(struct parser_state* state,
        libab_lexer_token to_consume) {
    libab_result result = LIBAB_SUCCESS;
    if(state->current_match == NULL) {
        result = LIBAB_EOF;
    } else if(state->current_match->type != to_consume) {
        result = LIBAB_UNEXPECTED;
    } else {
        _parser_state_step(state);
    }
    return result;
}

// Basic Tree Constructors

libab_result _parse_block(struct parser_state*, libab_tree**, int);
libab_result _parse_expression(struct parser_state* state, libab_tree** store_into);

libab_result _parser_allocate_node(libab_lexer_match* match, libab_tree** into) {
    libab_result result = LIBAB_SUCCESS;
    if(((*into) = malloc(sizeof(**into))) == NULL) {
        result = LIBAB_MALLOC;
    } else if(match) {
        (*into)->from = match->from;
        (*into)->to = match->to;
        (*into)->line = match->line;
        (*into)->line_from = match->line_from;
    }
    return result;
}

libab_result _parser_construct_node_string(struct parser_state* state, libab_lexer_match* match, libab_tree** into) {
    libab_result result = _parser_allocate_node(match, into);

    if(result == LIBAB_SUCCESS) {
        result = _parser_extract_token(state, &(*into)->string_value, match);
    }

    if(result != LIBAB_SUCCESS) {
        free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parser_construct_node_vec(libab_lexer_match* match, libab_tree** into) {
    libab_result result = _parser_allocate_node(match, into);

    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(vec_init(&(*into)->children));
    }

    if(result != LIBAB_SUCCESS) {
        free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parser_construct_node_both(struct parser_state* state, libab_lexer_match* match, libab_tree** store_into) {
    libab_result result = _parser_construct_node_string(state, match, store_into);
    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(vec_init(&(*store_into)->children));
        if(result != LIBAB_SUCCESS) {
            free((*store_into)->string_value);
            free(*store_into);
            *store_into = NULL;
        }
    }
    return result;
}

libab_result _parse_if(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* condition = NULL;
    libab_tree* if_branch = NULL;
    libab_tree* else_branch = NULL;

    if(_parser_is_type(state, TOKEN_KW_IF)) {
        result = _parser_construct_node_vec(state->current_match, store_into);
        if(result == LIBAB_SUCCESS) {
            (*store_into)->variant = IF;
        }
    }

    if(result == LIBAB_SUCCESS) {
        result = _parser_consume_type(state, TOKEN_KW_IF);
    }

    if(result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, '(');
    }

    if(result == LIBAB_SUCCESS) {
        result = _parse_expression(state, &condition);
    }

    if(result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ')');
    }

    if(result == LIBAB_SUCCESS) {
        result = _parse_expression(state, &if_branch);
    }

    if(result == LIBAB_SUCCESS) {
        if(_parser_is_type(state, TOKEN_KW_ELSE)) {
            _parser_state_step(state);
            result = _parse_expression(state, &else_branch);
        } else {
            result = _parser_allocate_node(state->last_match, &else_branch);
            if(result == LIBAB_SUCCESS) else_branch->variant = VOID;
        }
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(vec_add(&(*store_into)->children, condition));
        if(result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&(*store_into)->children, if_branch));
        }
        if(result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&(*store_into)->children, else_branch));
        }
    }

    if(result != LIBAB_SUCCESS) {
        if(condition) libab_tree_free_recursive(condition);
        if(if_branch) libab_tree_free_recursive(if_branch);
        if(else_branch) libab_tree_free_recursive(else_branch);

        libab_tree_free(*store_into);
        free(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parse_atom(struct parser_state* state, libab_tree** store_into) {
    libab_result result;
    if(_parser_is_type(state, TOKEN_NUM) || _parser_is_type(state, TOKEN_ID)) {
        result = _parser_construct_node_string(state, state->current_match, store_into);
        if(result == LIBAB_SUCCESS) {
            (*store_into)->variant = (state->current_match->type == TOKEN_NUM) ? NUM : ID;
        }
        _parser_state_step(state);
    } else if(_parser_is_type(state, TOKEN_KW_IF)) {
        result = _parse_if(state, store_into);
    } else if(_parser_is_char(state, '{')) {
        result = _parse_block(state, store_into, 1);
    } else {
        result = LIBAB_UNEXPECTED;
    }
    return result;
}

libab_result _parser_append_atom(struct parser_state* state, ll* append_to) {
    libab_result result;
    libab_tree* tree;
    result = _parse_atom(state, &tree);
    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(ll_append(append_to, tree));
        if(result != LIBAB_SUCCESS) {
            libab_tree_free_recursive(tree);
        }
    }
    return result;
}

libab_result _parser_construct_op(struct parser_state* state, libab_lexer_match* match, libab_tree** into) {
    libab_result result = _parser_construct_node_both(state, match, into);

    if(result == LIBAB_SUCCESS) {
        (*into)->variant = (match->type == TOKEN_OP_INFIX) ? OP : UNARY_OP;
    }

    return result;
}
libab_result _parser_append_op_node(struct parser_state* state, libab_lexer_match* match, ll* append_to) {
    libab_result result;
    libab_tree* new_tree = NULL;
    result = _parser_construct_op(state, match, &new_tree);
    if(result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(ll_append(append_to, new_tree));
        if(result != LIBAB_SUCCESS) {
            libab_tree_free(new_tree);
            free(new_tree);
        }
    }
    return result;
}


// Expression-specific utility functions

int _parser_match_is_op(libab_lexer_match* match) {
    return match->type == TOKEN_OP_INFIX ||
        match->type == TOKEN_OP_PREFIX ||
        match->type == TOKEN_OP_POSTFIX;
}

libab_result _parser_pop_brackets(struct parser_state* state, ll* pop_from, ll* push_to, char bracket, int* success) {
    libab_result result = LIBAB_SUCCESS;
    libab_lexer_match* remaining_match;
    while(result == LIBAB_SUCCESS && pop_from->tail && _parser_match_is_op(pop_from->tail->data)) {
        libab_lexer_match* new_match = ll_poptail(pop_from);
        result = _parser_append_op_node(state, new_match, push_to);
    }
    remaining_match = (pop_from->tail) ? pop_from->tail->data : NULL;
    *success = remaining_match && (remaining_match->type == TOKEN_CHAR) && (state->string[remaining_match->from] == bracket);
    return result;
}

enum parser_expression_type {
    EXPR_NONE,
    EXPR_ATOM,
    EXPR_OPEN_PARENTH,
    EXPR_CLOSE_PARENTH,
    EXPR_OP_PREFIX,
    EXPR_OP_POSTFIX,
    EXPR_OP_INFIX
};

int _parser_can_prefix_follow(enum parser_expression_type type) {
    return type == EXPR_OPEN_PARENTH || type == EXPR_OP_PREFIX || type == EXPR_OP_INFIX || type == EXPR_NONE;
}

int _parser_can_postfix_follow(enum parser_expression_type type) {
    return type == EXPR_CLOSE_PARENTH || type == EXPR_ATOM || type == EXPR_OP_POSTFIX;
}

libab_operator* _parser_find_operator(struct parser_state* state, libab_lexer_match* match) {
    char op_buffer[8];
    size_t token_size = match->to - match->from;
    size_t buffer_length = (token_size < 7) ? token_size : 7;
    strncpy(op_buffer, state->string + match->from, buffer_length);
    op_buffer[buffer_length] = '\0';
    return libab_table_search_operator(state->base_table, op_buffer);
}

libab_result _parser_expression_tree(struct parser_state* state, ll* source, libab_tree** into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* top = ll_poptail(source);
    if(top && top->variant == OP) {
        libab_tree* left = NULL;
        libab_tree* right = NULL;

        result = _parser_expression_tree(state, source, &right);
        if(result == LIBAB_SUCCESS) {
            result = _parser_expression_tree(state, source, &left);
        }

        if(result == LIBAB_SUCCESS && (left == NULL || right == NULL)) {
            result = LIBAB_UNEXPECTED;
        }

        if(result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&top->children, left));
        }
        if(result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&top->children, right));
        }

        if(result != LIBAB_SUCCESS) {
            if(left) libab_tree_free_recursive(left);
            if(right) libab_tree_free_recursive(right);
            libab_tree_free(top);
            free(top);
            top = NULL;
        }
    } else if(top && top->variant == UNARY_OP) {
        libab_tree* child = NULL;

        result = _parser_expression_tree(state, source, &child);

        if(result == LIBAB_SUCCESS && child == NULL) {
            result = LIBAB_UNEXPECTED;
        }

        if(result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&top->children, child));
        }

        if(result != LIBAB_SUCCESS) {
            if(child) libab_tree_free_recursive(child);
            libab_tree_free(top);
            free(top);
            top = NULL;
        }
    }
    *into = top;
    return result;
}

libab_result _parse_expression(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    ll out_stack;
    ll op_stack;
    int pop_success = 0;
    enum parser_expression_type last_type = EXPR_NONE;

    ll_init(&out_stack);
    ll_init(&op_stack);
    *store_into = NULL;

    while(result == LIBAB_SUCCESS && !_parser_eof(state)) {
        enum parser_expression_type new_type = EXPR_NONE;
        libab_lexer_match* new_token = state->current_match;
        char current_char = state->string[new_token->from];
        if(_parser_is_type(state, TOKEN_CHAR) && current_char != '{') {
            if(current_char == '(') {
                result = libab_convert_ds_result(ll_append(&op_stack, new_token));
                if(result != LIBAB_SUCCESS) break;
                _parser_state_step(state);
                new_type = EXPR_OPEN_PARENTH;
            } else if(current_char == ')') {
                result = _parser_pop_brackets(state, &op_stack, &out_stack, '(', &pop_success);
                if(result != LIBAB_SUCCESS || !pop_success) break;
                ll_poptail(&op_stack);
                _parser_state_step(state);
                new_type = EXPR_CLOSE_PARENTH;
            } else {
                break;
            }
        } else if(new_token->type == TOKEN_OP_PREFIX && _parser_can_prefix_follow(last_type)) {
            result = libab_convert_ds_result(ll_append(&op_stack, new_token));
            if(result != LIBAB_SUCCESS) break;
            _parser_state_step(state);
            new_type = EXPR_OP_PREFIX;
        } else if(new_token->type == TOKEN_OP_POSTFIX && _parser_can_postfix_follow(last_type)) {
            result = _parser_append_op_node(state, new_token, &out_stack);
            _parser_state_step(state);
            new_type = EXPR_OP_POSTFIX;
        } else if(new_token->type == TOKEN_OP_INFIX) {
            libab_operator* operator = _parser_find_operator(state, new_token);
            _parser_state_step(state);

            while(result == LIBAB_SUCCESS && op_stack.tail &&
                    _parser_match_is_op(op_stack.tail->data)) {
                libab_operator* other_operator = _parser_find_operator(state, new_token);

                if(new_token->type == TOKEN_OP_PREFIX ||
                        (operator->associativity == -1 &&
                         operator->precedence <= other_operator->precedence) ||
                        (operator->associativity == 1 &&
                         operator->precedence < other_operator->precedence)) {
                    libab_lexer_match* match = ll_poptail(&op_stack);
                    result = _parser_append_op_node(state, match, &out_stack);
                } else {
                    break;
                }
            }
            if(result == LIBAB_SUCCESS) {
                result = libab_convert_ds_result(ll_append(&op_stack, new_token));
            }
            new_type = EXPR_OP_INFIX;
        } else {
            if(last_type == EXPR_ATOM) break;
            result = _parser_append_atom(state, &out_stack);
            new_type = EXPR_ATOM;
        }
        last_type = new_type;
    }

    while(result == LIBAB_SUCCESS && op_stack.tail) {
        libab_lexer_match* match = ll_poptail(&op_stack);
        if(_parser_match_is_op(match)) {
            result = _parser_append_op_node(state, match, &out_stack);
        } else {
            result = LIBAB_UNEXPECTED;
        }
    }

    if(result == LIBAB_SUCCESS) {
        result = _parser_expression_tree(state, &out_stack, store_into);
    }

    if(result == LIBAB_SUCCESS && out_stack.tail) {
        libab_tree_free_recursive(*store_into);
        *store_into = NULL;
        result = LIBAB_UNEXPECTED;
    }

    if(result == LIBAB_SUCCESS && *store_into == NULL) {
        result = LIBAB_UNEXPECTED;
    }

    ll_free(&op_stack);
    ll_foreach(&out_stack, NULL, compare_always, _parser_foreach_free_tree);
    ll_free(&out_stack);

    return result;
}

libab_result _parse_block(struct parser_state* state,
        libab_tree** store_into, int expect_braces) {
    libab_result result;
    libab_tree* temp = NULL;
    result = _parser_construct_node_vec(state->current_match, store_into);
    if(result == LIBAB_SUCCESS) {
        (*store_into)->variant = BLOCK;
    }

    if(expect_braces && result == LIBAB_SUCCESS) result = _parser_consume_char(state, '{');

    while(result == LIBAB_SUCCESS && 
            !_parser_eof(state) &&
            !(expect_braces && _parser_is_char(state, '}'))) {
        result = _parse_expression(state, &temp);
        if(result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&(*store_into)->children, temp));
            if(result != LIBAB_SUCCESS) {
                libab_tree_free_recursive(temp);
            }
        }
        if(_parser_is_char(state, ';')) {
            temp = NULL;
            _parser_state_step(state);
        }
    }

    if(result == LIBAB_SUCCESS && temp == NULL) {
        result = _parser_allocate_node(state->last_match, &temp);
        if(result == LIBAB_SUCCESS) {
            temp->variant = VOID;
            result = libab_convert_ds_result(vec_add(&(*store_into)->children, temp));

            if(result != LIBAB_SUCCESS) {
                free(temp);
            }
        }
    }

    if(expect_braces && result == LIBAB_SUCCESS) result = _parser_consume_char(state, '}');

    if(result != LIBAB_SUCCESS && *store_into) {
        libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

void libab_parser_init(libab_parser* parser, libab_table* table) {
    parser->base_table = table;
}
libab_result libab_parser_parse(libab_parser* parser, ll* tokens,
        const char* string, libab_tree** store_into) {
    libab_result result;
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
