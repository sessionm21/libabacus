#include "libabacus.h"
#include <stdlib.h>
#include "util.h"
#include "reserved.h"

libab_result libab_init(libab* ab) {
    libab_result result;
    libab_table_init(&ab->table);
    libab_parser_init(&ab->parser, &ab->table);
    result = libab_lexer_init(&ab->lexer);

    if(result == LIBAB_SUCCESS) {
        result = libab_register_reserved_operators(&ab->lexer);
    }

    if(result != LIBAB_SUCCESS) {
        libab_table_free(&ab->table);
        libab_parser_free(&ab->parser);
        libab_lexer_free(&ab->lexer);
    }

    return result;
}

void _sanitize(char* to, const char* from, size_t buffer_size) {
    size_t index = 0;
    while(*from && index < (buffer_size - 2)) {
        if(*from == '+' || *from == '*' || *from == '\\') to[index++] = '\\';       
        to[index++] = *(from++);
    }
    to[index] = '\0';
}

libab_result _initialize_behavior(libab* ab, libab_behavior* behavior, 
        const char* type, libab_function_ptr func) {
    libab_result result = LIBAB_SUCCESS;
    ll tokens;

    ll_init(&tokens);

    result = libab_lexer_lex(&ab->lexer, type, &tokens);
    if(result == LIBAB_SUCCESS) {
        result = libab_parser_parse_type(&ab->parser, &tokens, type, &behavior->type);
    }

    ll_foreach(&tokens, NULL, compare_always, libab_lexer_foreach_match_free);
    ll_free(&tokens);

    return result;
}

libab_result _register_operator(libab* ab, const char* op, libab_operator_variant token_type, int precedence, int associativity, const char* type, libab_function_ptr func) {
    char op_buffer[8];
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_OP;
        new_entry->data_u.op.behavior.type = NULL;
        new_entry->data_u.op.precedence = precedence;
        new_entry->data_u.op.associativity = associativity;
        new_entry->data_u.op.type = token_type;
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = _initialize_behavior(ab, &(new_entry->data_u.op.behavior), type, func);
    }

    if(result == LIBAB_SUCCESS) {
        _sanitize(op_buffer, op, 8);
        result = libab_convert_lex_result(eval_config_add(&ab->lexer.config, op_buffer, TOKEN_OP));
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_table_put(&ab->table, op, new_entry);
    }

    if(result != LIBAB_SUCCESS) {
        if(new_entry && new_entry->data_u.op.behavior.type)
            libab_parsetype_free_recursive(new_entry->data_u.op.behavior.type);
        eval_config_remove(&ab->lexer.config, op, TOKEN_OP);
        free(new_entry);
    }

    return result;
}

libab_result libab_register_operator_infix(libab* ab, const char* op, int precedence, int associativity, const char* type, libab_function_ptr func) {
    return _register_operator(ab, op, OPERATOR_INFIX, precedence, associativity, type, func);
}

libab_result libab_register_operator_prefix(libab* ab, const char* op, const char* type, libab_function_ptr func) {
    return _register_operator(ab, op, OPERATOR_PREFIX, 0, 0, type, func);
}

libab_result libab_register_operator_postfix(libab* ab, const char* op, const char* type, libab_function_ptr func) {
    return _register_operator(ab, op, OPERATOR_POSTFIX, 0, 0, type, func);
}

libab_result libab_register_function(libab* ab, const char* name, const char* type, libab_function_ptr func) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_FUN;
        new_entry->data_u.function.behavior.type = NULL;
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = _initialize_behavior(ab, 
                &(new_entry->data_u.function.behavior), type, func);
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_table_put(&ab->table, name, new_entry);
    }

    if(result != LIBAB_SUCCESS) {
        if(new_entry && new_entry->data_u.function.behavior.type)
            libab_parsetype_free_recursive(new_entry->data_u.function.behavior.type);
        free(new_entry);
    }

    return result;
}

libab_result libab_free(libab* ab) {
    libab_table_free(&ab->table);
    libab_parser_free(&ab->parser);
    return libab_lexer_free(&ab->lexer);
}
