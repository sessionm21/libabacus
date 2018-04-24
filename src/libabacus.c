#include "libabacus.h"
#include "lexer.h"
#include "reserved.h"
#include "util.h"
#include <stdlib.h>

libab_result libab_init(libab* ab) {
    libab_ref null_ref;
    libab_result result;
    libab_ref_null(&null_ref);
    result = libab_create_table(&ab->table, &null_ref);

    if(result == LIBAB_SUCCESS) {
        libab_parser_init(&ab->parser, &ab->table);
        result = libab_lexer_init(&ab->lexer);
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_register_reserved_operators(&ab->lexer);
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_free(&ab->table);
        libab_parser_free(&ab->parser);
        libab_lexer_free(&ab->lexer);
    }
    libab_ref_free(&null_ref);

    return result;
}

void _sanitize(char* to, const char* from, size_t buffer_size) {
    size_t index = 0;
    while (*from && index < (buffer_size - 2)) {
        if (*from == '+' || *from == '*' || *from == '\\')
            to[index++] = '\\';
        to[index++] = *(from++);
    }
    to[index] = '\0';
}

void _initialize_behavior(libab* ab, libab_behavior* behavior, libab_ref* type,
                          libab_function_ptr func) {
    behavior->impl.variant = BIMPL_INTERNAL;
    behavior->impl.data_u.internal = func;
    libab_ref_copy(type, &behavior->type);
}

libab_result _register_operator(libab* ab, const char* op,
                                libab_operator_variant token_type,
                                int precedence, int associativity,
                                libab_ref* type, libab_function_ptr func) {
    char op_buffer[8];
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if ((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_OP;
        new_entry->data_u.op.precedence = precedence;
        new_entry->data_u.op.associativity = associativity;
        new_entry->data_u.op.type = token_type;
        _initialize_behavior(ab, &(new_entry->data_u.op.behavior), type, func);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        _sanitize(op_buffer, op, 8);
        result = libab_convert_lex_result(
            eval_config_add(&ab->lexer.config, op_buffer, TOKEN_OP));
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_table_put(libab_ref_get(&ab->table), op, new_entry);
    }

    if (result != LIBAB_SUCCESS) {
        if (new_entry)
            libab_ref_free(&new_entry->data_u.op.behavior.type);
        eval_config_remove(&ab->lexer.config, op, TOKEN_OP);
        free(new_entry);
    }

    return result;
}

libab_result libab_register_operator_infix(libab* ab, const char* op,
                                           int precedence, int associativity,
                                           libab_ref* type,
                                           libab_function_ptr func) {
    return _register_operator(ab, op, OPERATOR_INFIX, precedence, associativity,
                              type, func);
}

libab_result libab_register_operator_prefix(libab* ab, const char* op,
                                            libab_ref* type,
                                            libab_function_ptr func) {
    return _register_operator(ab, op, OPERATOR_PREFIX, 0, 0, type, func);
}

libab_result libab_register_operator_postfix(libab* ab, const char* op,
                                             libab_ref* type,
                                             libab_function_ptr func) {
    return _register_operator(ab, op, OPERATOR_POSTFIX, 0, 0, type, func);
}

libab_result libab_register_function(libab* ab, const char* name,
                                     libab_ref* type, libab_function_ptr func) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if ((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_FUN;
        _initialize_behavior(ab, &(new_entry->data_u.function.behavior), type,
                             func);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_table_put(libab_ref_get(&ab->table), name, new_entry);
    }

    if (result != LIBAB_SUCCESS) {
        if (new_entry)
            libab_ref_free(&new_entry->data_u.function.behavior.type);
        free(new_entry);
    }

    return result;
}

libab_result libab_register_basetype(libab* ab, const char* name,
                                     libab_basetype* basetype) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if ((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_BASETYPE;
        new_entry->data_u.basetype = basetype;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_table_put(libab_ref_get(&ab->table), name, new_entry);
    }

    if (result != LIBAB_SUCCESS) {
        free(new_entry);
    }

    return result;
}

libab_result libab_create_type(libab* ab, libab_ref* into, const char* type) {
    libab_result result;
    ll tokens;
    ll_init(&tokens);
    result = libab_lexer_lex(&ab->lexer, type, &tokens);
    if (result == LIBAB_SUCCESS) {
        result = libab_parser_parse_type(&ab->parser, &tokens, type, into);
    }
    ll_foreach(&tokens, NULL, compare_always, libab_lexer_foreach_match_free);
    ll_free(&tokens);
    return result;
}

libab_result libab_free(libab* ab) {
    libab_ref_free(&ab->table);
    libab_parser_free(&ab->parser);
    return libab_lexer_free(&ab->lexer);
}
