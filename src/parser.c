#include "parser.h"
#include "lexer.h"
#include "reserved.h"
#include "result.h"
#include "util.h"
#include <stdlib.h>
#include <string.h>

struct parser_state {
    ll_node* current_node;
    libab_lexer_match* current_match;
    libab_lexer_match* last_match;
    const char* string;
    libab_table* base_table;
};

struct operator_data {
    int associativity;
    int precedence;
};

/* Utilities */
#define PARSE_CHILD(result, state, parse_function, parse_into, into)           \
    do {                                                                       \
        result = parse_function(state, &parse_into);                           \
        if (result == LIBAB_SUCCESS) {                                         \
            result = libab_convert_ds_result(vec_add(into, parse_into));       \
            if (result != LIBAB_SUCCESS) {                                     \
                libab_tree_free_recursive(parse_into);                         \
            }                                                                  \
        }                                                                      \
    } while (0);

void _parser_extract_token_buffer(struct parser_state* state, char* buffer,
                                  size_t length, libab_lexer_match* match) {
    size_t token_size = match->to - match->from;
    size_t buffer_length =
        (token_size < (length - 1)) ? token_size : (length - 1);
    strncpy(buffer, state->string + match->from, buffer_length);
    buffer[buffer_length] = '\0';
}

int _parser_foreach_free_tree(void* data, va_list args) {
    libab_tree_free(data);
    free(data);
    return 0;
}

libab_result _parser_extract_token(struct parser_state* state, char** into,
                                   libab_lexer_match* match) {
    return libab_copy_string_range(into, state->string, match->from, match->to);
}

/* State functions */
void _parser_state_update(struct parser_state* state) {
    state->current_match =
        state->current_node ? state->current_node->data : NULL;
    if (state->current_match)
        state->last_match = state->current_match;
}

void _parser_state_init(struct parser_state* state, ll* tokens,
                        const char* string, libab_table* table) {
    state->last_match = NULL;
    state->current_node = tokens->head;
    state->string = string;
    state->base_table = table;
    _parser_state_update(state);
}

void _parser_state_step(struct parser_state* state) {
    if (state->current_node) {
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
    if (state->current_match == NULL) {
        result = LIBAB_EOF;
    } else if (state->current_match->type != TOKEN_CHAR ||
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
    if (state->current_match == NULL) {
        result = LIBAB_EOF;
    } else if (state->current_match->type != to_consume) {
        result = LIBAB_UNEXPECTED;
    } else {
        _parser_state_step(state);
    }
    return result;
}

/* Basic Tree Constructors */

libab_result _parse_block(struct parser_state*, libab_tree**, int);
libab_result _parse_expression(struct parser_state* state,
                               libab_tree** store_into);
libab_result _parse_type(struct parser_state* state, libab_ref* ref);

libab_result _parse_braced_block(struct parser_state* state,
                                 libab_tree** store_into) {
    return _parse_block(state, store_into, 1);
}

libab_result _parser_allocate_type(libab_parsetype** into, const char* source,
                                   size_t from, size_t to) {
    libab_result result = LIBAB_SUCCESS;
    if ((*into = malloc(sizeof(**into)))) {
        (*into)->variant = 0;
        result =
            libab_copy_string_range(&(*into)->data_u.name, source, from, to);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        free(*into);
        *into = NULL;
    }
    return result;
}

libab_result _parser_append_type(struct parser_state* state,
                                 libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref temp;
    result = _parse_type(state, &temp);
    if (result == LIBAB_SUCCESS) {
        result = libab_ref_vec_insert(into, &temp);
        libab_ref_free(&temp);
    }
    return result;
}

libab_result _parse_type_list(struct parser_state* state, libab_ref_vec* into,
                              char end_char) {
    libab_result result = LIBAB_SUCCESS;
    int is_parenth, is_comma;
    while (result == LIBAB_SUCCESS && !_parser_eof(state) &&
           !_parser_is_char(state, end_char)) {
        result = _parser_append_type(state, into);
        is_parenth = _parser_is_char(state, end_char);
        is_comma = _parser_is_char(state, ',');
        if (result == LIBAB_SUCCESS && !(is_parenth || is_comma)) {
            result = LIBAB_UNEXPECTED;
        } else if (result == LIBAB_SUCCESS && is_comma) {
            _parser_state_step(state);
            if (_parser_is_char(state, end_char)) {
                result = LIBAB_UNEXPECTED;
            }
        }
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, end_char);
    }

    return result;
}

libab_result _parse_type_id(struct parser_state* state,
                            libab_parsetype** into) {
    libab_result result;
    int placeholder_flag = 0;
    *into = NULL;

    if (_parser_is_char(state, '\'')) {
        placeholder_flag = LIBABACUS_TYPE_F_PLACE;
        _parser_state_step(state);
    }

    if (_parser_is_type(state, TOKEN_ID)) {
        result = _parser_allocate_type(into, state->string,
                                       state->current_match->from,
                                       state->current_match->to);
    } else {
        result = _parser_consume_type(state, TOKEN_ID);
    }

    if (result == LIBAB_SUCCESS) {
        (*into)->variant |= placeholder_flag;
        _parser_state_step(state);
    }

    if (result == LIBAB_SUCCESS && _parser_is_char(state, '(')) {
        if (placeholder_flag) {
            result = LIBAB_UNEXPECTED;
        } else {
            result = libab_ref_vec_init(&(*into)->children);
            if (result != LIBAB_SUCCESS) {
                free((*into)->data_u.name);
                free(*into);
                *into = NULL;
            } else {
                (*into)->variant |= LIBABACUS_TYPE_F_PARENT;
                _parser_state_step(state);
                result = _parse_type_list(state, &(*into)->children, ')');
            }
        }
    }

    if (result != LIBAB_SUCCESS && *into) {
        libab_parsetype_free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parse_type_function(struct parser_state* state,
                                  libab_parsetype** into) {
    libab_result result = _parser_allocate_type(into, "function", 0, 8);
    if (result == LIBAB_SUCCESS) {
        (*into)->variant |= LIBABACUS_TYPE_F_PARENT;
        result = libab_ref_vec_init(&(*into)->children);
        if (result != LIBAB_SUCCESS) {
            free((*into)->data_u.name);
            free(*into);
            *into = NULL;
        } else {
            _parser_state_step(state);
        }
    }

    if (result == LIBAB_SUCCESS) {
        result = _parse_type_list(state, &(*into)->children, ')');
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_type(state, TOKEN_KW_ARROW);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_append_type(state, &(*into)->children);
    }

    if (result != LIBAB_SUCCESS && *into) {
        libab_parsetype_free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parse_type_array(struct parser_state* state,
                               libab_parsetype** into) {
    libab_result result = _parser_allocate_type(into, "array", 0, 5);
    if (result == LIBAB_SUCCESS) {
        (*into)->variant |= LIBABACUS_TYPE_F_PARENT;
        result = libab_ref_vec_init(&(*into)->children);
        if (result != LIBAB_SUCCESS) {
            free((*into)->data_u.name);
            free(*into);
            *into = NULL;
        } else {
            _parser_state_step(state);
        }
    }
    if (result == LIBAB_SUCCESS) {
        result = _parser_append_type(state, &(*into)->children);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ']');
    }

    if (result != LIBAB_SUCCESS && *into) {
        libab_parsetype_free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parse_type_raw(struct parser_state* state,
                             libab_parsetype** into) {
    libab_result result;
    if (_parser_is_type(state, TOKEN_ID) || _parser_is_char(state, '\'')) {
        result = _parse_type_id(state, into);
    } else if (_parser_is_char(state, '(')) {
        result = _parse_type_function(state, into);
    } else if (_parser_is_char(state, '[')) {
        result = _parse_type_array(state, into);
    } else {
        *into = NULL;
        result = LIBAB_UNEXPECTED;
    }
    return result;
}

void _parse_type_free(void* data) {
    libab_parsetype_free(data);
    free(data);
}

libab_result _parse_type(struct parser_state* state, libab_ref* into) {
    libab_parsetype* store_into;
    libab_result result = _parse_type_raw(state, &store_into);
    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, store_into, _parse_type_free);
        if (result != LIBAB_SUCCESS) {
            libab_parsetype_free(store_into);
        }
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

libab_result _parser_allocate_node(libab_lexer_match* match,
                                   libab_tree** into) {
    libab_result result = LIBAB_SUCCESS;
    if (((*into) = malloc(sizeof(**into))) == NULL) {
        result = LIBAB_MALLOC;
    } else if (match) {
        (*into)->from = match->from;
        (*into)->to = match->to;
        (*into)->line = match->line;
        (*into)->line_from = match->line_from;
    }
    return result;
}

libab_result _parser_construct_node_string(struct parser_state* state,
                                           libab_lexer_match* match,
                                           libab_tree** into) {
    libab_result result = _parser_allocate_node(match, into);

    if (result == LIBAB_SUCCESS) {
        result = _parser_extract_token(state, &(*into)->string_value, match);
    }

    if (result != LIBAB_SUCCESS) {
        free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parser_construct_node_vec(libab_lexer_match* match,
                                        libab_tree** into) {
    libab_result result = _parser_allocate_node(match, into);

    if (result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(vec_init(&(*into)->children));
    }

    if (result != LIBAB_SUCCESS) {
        free(*into);
        *into = NULL;
    }

    return result;
}

libab_result _parser_construct_node_both(struct parser_state* state,
                                         libab_lexer_match* match,
                                         libab_tree** store_into) {
    libab_result result =
        _parser_construct_node_string(state, match, store_into);
    if (result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(vec_init(&(*store_into)->children));
        if (result != LIBAB_SUCCESS) {
            free((*store_into)->string_value);
            free(*store_into);
            *store_into = NULL;
        }
    }
    return result;
}

libab_result _parse_void(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    if ((*store_into = malloc(sizeof(**store_into)))) {
        (*store_into)->variant = TREE_VOID;
    } else {
        result = LIBAB_MALLOC;
    }
    return result;
}

libab_result _parse_if(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* condition = NULL;
    libab_tree* if_branch = NULL;
    libab_tree* else_branch = NULL;

    if (_parser_is_type(state, TOKEN_KW_IF)) {
        result = _parser_construct_node_vec(state->current_match, store_into);
        if (result == LIBAB_SUCCESS) {
            (*store_into)->variant = TREE_IF;
            _parser_state_step(state);
        }
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, '(');
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, condition,
                    &(*store_into)->children);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ')');
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, if_branch,
                    &(*store_into)->children);
    }

    if (result == LIBAB_SUCCESS) {
        if (_parser_is_type(state, TOKEN_KW_ELSE)) {
            _parser_state_step(state);
            PARSE_CHILD(result, state, _parse_expression, else_branch,
                        &(*store_into)->children);
        } else {
            PARSE_CHILD(result, state, _parse_void, else_branch,
                        &(*store_into)->children);
        }
    }

    if (result != LIBAB_SUCCESS) {
        if (*store_into)
            libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parse_fun_param(struct parser_state* state,
                              libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    if (_parser_is_type(state, TOKEN_ID)) {
        result = _parser_construct_node_string(state, state->current_match,
                                               store_into);
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS) {
        _parser_state_step(state);
        (*store_into)->variant = TREE_FUN_PARAM;
        libab_ref_null(&(*store_into)->type);
        result = _parser_consume_char(state, ':');
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&(*store_into)->type);
        result = _parse_type(state, &(*store_into)->type);
    }

    if (result != LIBAB_SUCCESS && *store_into) {
        libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }
    return result;
}

libab_result _parse_def_fun(struct parser_state* state,
                            libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* temp;
    if (_parser_is_type(state, TOKEN_KW_LET)) {
        _parser_state_step(state);
        if (!_parser_eof(state)) {
            result = _parser_construct_node_both(state, state->current_match,
                                                 store_into);
        } else {
            result = LIBAB_UNEXPECTED;
        }
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS) {
        _parser_state_step(state);
        libab_ref_null(&(*store_into)->type);
        (*store_into)->variant = TREE_FUN;
        result = _parser_consume_char(state, ':');
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&(*store_into)->type);
        result = _parse_type(state, &(*store_into)->type);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_type(state, TOKEN_KW_BE);
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, temp,
                    &(*store_into)->children);
    }

    if (result != LIBAB_SUCCESS && *store_into) {
        libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parse_fun(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    int is_parenth, is_comma;
    libab_tree* temp;
    result = _parser_consume_type(state, TOKEN_KW_FUN);
    if (result == LIBAB_SUCCESS) {
        if (_parser_is_type(state, TOKEN_ID)) {
            result = _parser_construct_node_both(state, state->current_match,
                                                 store_into);
        } else {
            result = LIBAB_UNEXPECTED;
        }
    }
    if (result == LIBAB_SUCCESS) {
        _parser_state_step(state);
        libab_ref_null(&(*store_into)->type);
        (*store_into)->variant = TREE_FUN;
        result = _parser_consume_char(state, '(');
    }
    while (result == LIBAB_SUCCESS && !_parser_eof(state) &&
           !_parser_is_char(state, ')')) {
        PARSE_CHILD(result, state, _parse_fun_param, temp,
                    &(*store_into)->children);
        is_parenth = _parser_is_char(state, ')');
        is_comma = _parser_is_char(state, ',');
        if (result == LIBAB_SUCCESS && !(is_parenth || is_comma)) {
            result = LIBAB_UNEXPECTED;
        } else if (result == LIBAB_SUCCESS && is_comma) {
            _parser_state_step(state);
            if (_parser_is_char(state, ')')) {
                result = LIBAB_UNEXPECTED;
            }
        }
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ')');
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ':');
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&(*store_into)->type);
        result = _parse_type(state, &(*store_into)->type);
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_braced_block, temp,
                    &(*store_into)->children);
    }

    if (result != LIBAB_SUCCESS && *store_into) {
        libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parse_return(struct parser_state* state,
                           libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* child = NULL;
    if (_parser_is_type(state, TOKEN_KW_RETURN)) {
        result = _parser_construct_node_vec(state->current_match, store_into);
        if (result == LIBAB_SUCCESS) {
            (*store_into)->variant = TREE_RETURN;
            _parser_state_step(state);
        }
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, child,
                    &(*store_into)->children);
    }

    if (result != LIBAB_SUCCESS) {
        if (*store_into)
            libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parse_while(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* condition = NULL;
    libab_tree* value = NULL;

    if (_parser_is_type(state, TOKEN_KW_WHILE)) {
        result = _parser_construct_node_vec(state->current_match, store_into);
        if (result == LIBAB_SUCCESS) {
            (*store_into)->variant = TREE_WHILE;
            _parser_state_step(state);
        }
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, '(');
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, condition,
                    &(*store_into)->children);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ')');
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, value,
                    &(*store_into)->children);
    }

    if (result != LIBAB_SUCCESS) {
        if (*store_into)
            libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parse_dowhile(struct parser_state* state,
                            libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* value = NULL;
    libab_tree* condition = NULL;

    if (_parser_is_type(state, TOKEN_KW_DO)) {
        result = _parser_construct_node_vec(state->current_match, store_into);
        if (result == LIBAB_SUCCESS) {
            (*store_into)->variant = TREE_DOWHILE;
            _parser_state_step(state);
        }
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, value,
                    &(*store_into)->children);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_type(state, TOKEN_KW_WHILE);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, '(');
    }

    if (result == LIBAB_SUCCESS) {
        PARSE_CHILD(result, state, _parse_expression, condition,
                    &(*store_into)->children);
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ')');
    }

    return result;
}

libab_result _parse_call(struct parser_state* state, libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* temp;

    if (_parser_is_char(state, '(')) {
        result = _parser_construct_node_vec(state->current_match, store_into);
        if (result == LIBAB_SUCCESS) {
            (*store_into)->variant = TREE_CALL;
        }
        _parser_state_step(state);
    } else {
        result = LIBAB_UNEXPECTED;
    }

    while (result == LIBAB_SUCCESS && !_parser_eof(state) &&
           !_parser_is_char(state, ')')) {
        PARSE_CHILD(result, state, _parse_expression, temp,
                    &(*store_into)->children);

        if (result == LIBAB_SUCCESS &&
            !(_parser_is_char(state, ')') || _parser_is_char(state, ','))) {
            result = LIBAB_UNEXPECTED;
        } else if (_parser_is_char(state, ',')) {
            _parser_state_step(state);
        }
    }
    if (result == LIBAB_SUCCESS) {
        result = _parser_consume_char(state, ')');
    }

    if (result != LIBAB_SUCCESS) {
        if (*store_into)
            libab_tree_free_recursive(*store_into);
        *store_into = NULL;
    }

    return result;
}

libab_result _parser_append_call(struct parser_state* state, ll* append_to) {
    libab_result result;
    libab_tree* into;
    result = _parse_call(state, &into);
    if (result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(ll_append(append_to, into));
        if (result != LIBAB_SUCCESS) {
            libab_tree_free_recursive(into);
        }
    }
    return result;
}

libab_result _parse_atom(struct parser_state* state, libab_tree** store_into) {
    libab_result result;
    if (_parser_is_type(state, TOKEN_NUM) || _parser_is_type(state, TOKEN_ID)) {
        result = _parser_construct_node_string(state, state->current_match,
                                               store_into);
        if (result == LIBAB_SUCCESS) {
            (*store_into)->variant =
                (state->current_match->type == TOKEN_NUM) ? TREE_NUM : TREE_ID;
        }
        _parser_state_step(state);
    } else if (_parser_is_type(state, TOKEN_KW_IF)) {
        result = _parse_if(state, store_into);
    } else if (_parser_is_type(state, TOKEN_KW_WHILE)) {
        result = _parse_while(state, store_into);
    } else if (_parser_is_type(state, TOKEN_KW_DO)) {
        result = _parse_dowhile(state, store_into);
    } else if (_parser_is_char(state, '{')) {
        result = _parse_braced_block(state, store_into);
    } else if (_parser_is_type(state, TOKEN_KW_FUN)) {
        result = _parse_fun(state, store_into);
    } else if (_parser_is_type(state, TOKEN_KW_LET)) {
        result = _parse_def_fun(state, store_into);
    } else if (_parser_is_type(state, TOKEN_KW_RETURN)) {
        result = _parse_return(state, store_into);
    } else {
        result = LIBAB_UNEXPECTED;
    }
    return result;
}

libab_result _parser_append_atom(struct parser_state* state, ll* append_to) {
    libab_result result;
    libab_tree* tree;
    result = _parse_atom(state, &tree);
    if (result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(ll_append(append_to, tree));
        if (result != LIBAB_SUCCESS) {
            libab_tree_free_recursive(tree);
        }
    }
    return result;
}

libab_result _parser_construct_op(struct parser_state* state,
                                  libab_lexer_match* match, libab_tree** into) {
    libab_result result = _parser_construct_node_both(state, match, into);

    if (result == LIBAB_SUCCESS) {
        if (match->type == TOKEN_OP_INFIX) {
            (*into)->variant = TREE_OP;
        } else if (match->type == TOKEN_OP_RESERVED) {
            (*into)->variant = TREE_RESERVED_OP;
        } else if (match->type == TOKEN_OP_PREFIX) {
            (*into)->variant = TREE_PREFIX_OP;
        } else {
            (*into)->variant = TREE_POSTFIX_OP;
        }
    }

    return result;
}
libab_result _parser_append_op_node(struct parser_state* state,
                                    libab_lexer_match* match, ll* append_to) {
    libab_result result;
    libab_tree* new_tree = NULL;
    result = _parser_construct_op(state, match, &new_tree);
    if (result == LIBAB_SUCCESS) {
        result = libab_convert_ds_result(ll_append(append_to, new_tree));
        if (result != LIBAB_SUCCESS) {
            libab_tree_free(new_tree);
            free(new_tree);
        }
    }
    return result;
}

/* Expression-specific utility functions */

int _parser_match_is_op(libab_lexer_match* match) {
    return match->type == TOKEN_OP || match->type == TOKEN_OP_INFIX ||
           match->type == TOKEN_OP_PREFIX || match->type == TOKEN_OP_POSTFIX ||
           match->type == TOKEN_OP_RESERVED;
}

libab_result _parser_pop_brackets(struct parser_state* state, ll* pop_from,
                                  ll* push_to, char bracket, int* success) {
    libab_result result = LIBAB_SUCCESS;
    libab_lexer_match* remaining_match;
    while (result == LIBAB_SUCCESS && pop_from->tail &&
           _parser_match_is_op(pop_from->tail->data)) {
        libab_lexer_match* new_match = ll_poptail(pop_from);
        result = _parser_append_op_node(state, new_match, push_to);
    }
    remaining_match = (pop_from->tail) ? pop_from->tail->data : NULL;
    *success = remaining_match && (remaining_match->type == TOKEN_CHAR) &&
               (state->string[remaining_match->from] == bracket);
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
    return type == EXPR_OPEN_PARENTH || type == EXPR_OP_PREFIX ||
           type == EXPR_OP_INFIX || type == EXPR_NONE;
}

int _parser_can_postfix_follow(enum parser_expression_type type) {
    return type == EXPR_CLOSE_PARENTH || type == EXPR_ATOM ||
           type == EXPR_OP_POSTFIX;
}

int _parser_can_atom_follow(enum parser_expression_type type) {
    return !(type == EXPR_CLOSE_PARENTH || type == EXPR_OP_POSTFIX ||
             type == EXPR_ATOM);
}

void _parser_find_operator_infix(struct parser_state* state,
                                 libab_lexer_match* match,
                                 struct operator_data* data) {
    char op_buffer[8];
    _parser_extract_token_buffer(state, op_buffer, 8, match);
    if (match->type != TOKEN_OP_RESERVED) {
        libab_operator* operator= libab_table_search_operator(
            state->base_table, op_buffer, OPERATOR_INFIX);
        data->associativity = operator->associativity;
        data->precedence = operator->precedence;
    } else {
        const libab_reserved_operator* operator=
            libab_find_reserved_operator(op_buffer);
        data->associativity = operator->associativity;
        data->precedence = operator->precedence;
    }
}

libab_result _parser_expression_tree(struct parser_state* state, ll* source,
                                     libab_tree** into) {
    libab_result result = LIBAB_SUCCESS;
    libab_tree* top = ll_poptail(source);
    if (top == NULL) {
        result = LIBAB_UNEXPECTED;
    } else if (top->variant == TREE_OP || top->variant == TREE_RESERVED_OP) {
        libab_tree* left = NULL;
        libab_tree* right = NULL;

        result = _parser_expression_tree(state, source, &right);
        if (result == LIBAB_SUCCESS) {
            result = _parser_expression_tree(state, source, &left);
        }

        if (result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&top->children, left));
        }
        if (result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&top->children, right));
        }

        if (result != LIBAB_SUCCESS) {
            if (left)
                libab_tree_free_recursive(left);
            if (right)
                libab_tree_free_recursive(right);
            libab_tree_free(top);
            free(top);
            top = NULL;
        }
    } else if (top->variant == TREE_PREFIX_OP ||
               top->variant == TREE_POSTFIX_OP || top->variant == TREE_CALL) {
        libab_tree* child = NULL;

        result = _parser_expression_tree(state, source, &child);

        if (result == LIBAB_SUCCESS) {
            result = libab_convert_ds_result(vec_add(&top->children, child));
        }

        if (result != LIBAB_SUCCESS) {
            if (child)
                libab_tree_free_recursive(child);
            if (top->variant == TREE_PREFIX_OP ||
                top->variant == TREE_POSTFIX_OP) {
                libab_tree_free(top);
                free(top);
            } else {
                libab_tree_free_recursive(top);
            }
            top = NULL;
        }
    }
    *into = top;
    return result;
}

int _parser_match_is_postfix_op(struct parser_state* state,
                                libab_lexer_match* match) {
    int is_postfix = 0;
    libab_operator* operator;
    char op_buffer[8];
    _parser_extract_token_buffer(state, op_buffer, 8, match);
    operator= libab_table_search_operator(state->base_table, op_buffer,
                                          OPERATOR_POSTFIX);
    if (operator)
        is_postfix = 1;
    return is_postfix;
}

int _parser_match_is_prefix_op(struct parser_state* state,
                               libab_lexer_match* match) {
    int is_prefix = 0;
    libab_operator* operator;
    char op_buffer[8];
    _parser_extract_token_buffer(state, op_buffer, 8, match);
    operator= libab_table_search_operator(state->base_table, op_buffer,
                                          OPERATOR_PREFIX);
    if (operator)
        is_prefix = 1;
    return is_prefix;
}

int _parser_match_is_infix_op(struct parser_state* state,
                              libab_lexer_match* match) {
    int is_infix = 0;
    if (match->type == TOKEN_OP_RESERVED) {
        is_infix = 1;
    } else {
        libab_operator* operator;
        char op_buffer[8];
        _parser_extract_token_buffer(state, op_buffer, 8, match);
        operator= libab_table_search_operator(state->base_table, op_buffer,
                                              OPERATOR_INFIX);
        if (operator)
            is_infix = 1;
    }
    return is_infix;
}

libab_result _parse_expression(struct parser_state* state,
                               libab_tree** store_into) {
    libab_result result = LIBAB_SUCCESS;
    struct operator_data operator;
    struct operator_data other_operator;
    ll out_stack;
    ll op_stack;
    int pop_success = 0;
    enum parser_expression_type last_type = EXPR_NONE;

    ll_init(&out_stack);
    ll_init(&op_stack);
    *store_into = NULL;

    while (result == LIBAB_SUCCESS && !_parser_eof(state)) {
        enum parser_expression_type new_type = EXPR_NONE;
        libab_lexer_match* new_token = state->current_match;
        char current_char = state->string[new_token->from];
        if (_parser_is_type(state, TOKEN_CHAR) && current_char != '{') {
            if (current_char == '(' && _parser_can_postfix_follow(last_type)) {
                result = _parser_append_call(state, &out_stack);
                if (result != LIBAB_SUCCESS)
                    break;
                new_type = EXPR_OP_POSTFIX;
            } else if (current_char == '(') {
                result =
                    libab_convert_ds_result(ll_append(&op_stack, new_token));
                if (result != LIBAB_SUCCESS)
                    break;
                _parser_state_step(state);
                new_type = EXPR_OPEN_PARENTH;
            } else if (current_char == ')') {
                result = _parser_pop_brackets(state, &op_stack, &out_stack, '(',
                                              &pop_success);
                if (result != LIBAB_SUCCESS || !pop_success)
                    break;
                ll_poptail(&op_stack);
                _parser_state_step(state);
                new_type = EXPR_CLOSE_PARENTH;
            } else {
                break;
            }
        } else if (_parser_match_is_prefix_op(state, new_token) &&
                   _parser_can_prefix_follow(last_type)) {
            new_token->type = TOKEN_OP_PREFIX;
            result = libab_convert_ds_result(ll_append(&op_stack, new_token));
            if (result != LIBAB_SUCCESS)
                break;
            _parser_state_step(state);
            new_type = EXPR_OP_PREFIX;
        } else if (_parser_match_is_postfix_op(state, new_token) &&
                   _parser_can_postfix_follow(last_type)) {
            new_token->type = TOKEN_OP_POSTFIX;
            result = _parser_append_op_node(state, new_token, &out_stack);
            _parser_state_step(state);
            new_type = EXPR_OP_POSTFIX;
        } else if (_parser_match_is_infix_op(state, new_token)) {
            if (new_token->type == TOKEN_OP)
                new_token->type = TOKEN_OP_INFIX;
            _parser_find_operator_infix(state, new_token, &operator);
            _parser_state_step(state);

            while (result == LIBAB_SUCCESS && op_stack.tail &&
                   _parser_match_is_op(op_stack.tail->data)) {
                _parser_find_operator_infix(state, op_stack.tail->data,
                                            &other_operator);

                if (((libab_lexer_match*)op_stack.tail->data)->type ==
                        TOKEN_OP_PREFIX ||
                    (operator.associativity == - 1 &&
                     operator.precedence <= other_operator.precedence) ||
                    (operator.associativity == 1 &&
                     operator.precedence<other_operator.precedence)) {
                    libab_lexer_match* match = ll_poptail(&op_stack);
                    result = _parser_append_op_node(state, match, &out_stack);
                } else {
                    break;
                }
            }
            if (result == LIBAB_SUCCESS) {
                result =
                    libab_convert_ds_result(ll_append(&op_stack, new_token));
            }
            new_type = EXPR_OP_INFIX;
        } else {
            if (!_parser_can_atom_follow(last_type))
                break;
            result = _parser_append_atom(state, &out_stack);
            new_type = EXPR_ATOM;
        }
        last_type = new_type;
    }

    while (result == LIBAB_SUCCESS && op_stack.tail) {
        libab_lexer_match* match = ll_poptail(&op_stack);
        if (_parser_match_is_op(match)) {
            result = _parser_append_op_node(state, match, &out_stack);
        } else {
            result = LIBAB_UNEXPECTED;
        }
    }

    if (result == LIBAB_SUCCESS) {
        result = _parser_expression_tree(state, &out_stack, store_into);
    }

    if (result == LIBAB_SUCCESS && out_stack.tail) {
        libab_tree_free_recursive(*store_into);
        *store_into = NULL;
        result = LIBAB_UNEXPECTED;
    }

    if (result == LIBAB_SUCCESS && *store_into == NULL) {
        result = LIBAB_UNEXPECTED;
    }

    ll_free(&op_stack);
    ll_foreach(&out_stack, NULL, compare_always, _parser_foreach_free_tree);
    ll_free(&out_stack);

    return result;
}

libab_result _parse_block(struct parser_state* state, libab_tree** store_into,
                          int expect_braces) {
    libab_result result;
    libab_tree* temp = NULL;
    result = _parser_construct_node_vec(state->current_match, store_into);
    if (result == LIBAB_SUCCESS) {
        (*store_into)->variant = TREE_BLOCK;
    }

    if (expect_braces && result == LIBAB_SUCCESS)
        result = _parser_consume_char(state, '{');

    while (result == LIBAB_SUCCESS && !_parser_eof(state) &&
           !(expect_braces && _parser_is_char(state, '}'))) {
        PARSE_CHILD(result, state, _parse_expression, temp,
                    &(*store_into)->children);
        if (_parser_is_char(state, ';')) {
            temp = NULL;
            _parser_state_step(state);
        }
    }

    if (result == LIBAB_SUCCESS && temp == NULL) {
        PARSE_CHILD(result, state, _parse_void, temp, &(*store_into)->children);
    }

    if (expect_braces && result == LIBAB_SUCCESS)
        result = _parser_consume_char(state, '}');

    if (result != LIBAB_SUCCESS && *store_into) {
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
    if (result == LIBAB_SUCCESS) {
        (*store_into)->variant = TREE_BASE;
    }

    return result;
}
libab_result libab_parser_parse_type(libab_parser* parser, ll* tokens,
                                     const char* string,
                                     libab_ref* store_into) {
    struct parser_state state;
    _parser_state_init(&state, tokens, string, parser->base_table);

    return _parse_type(&state, store_into);
}
void libab_parser_free(libab_parser* parser) { parser->base_table = NULL; }
