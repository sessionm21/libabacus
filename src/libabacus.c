#include "libabacus.h"
#include <stdlib.h>
#include "util.h"

libab_result libab_init(libab* ab) {
    libab_table_init(&ab->table);
    libab_parser_init(&ab->parser, &ab->table);
    return libab_lexer_init(&ab->lexer);
}

libab_result _register_operator(libab* ab, const char* op, int token_type, int precedence, libab_function_ptr func) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_OP;
        new_entry->data_u.op.function = func;
        new_entry->data_u.op.precedence = precedence;
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_convert_lex_result(eval_config_add(&ab->lexer.config, op, token_type));
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_table_put(&ab->table, op, new_entry);
    }

    if(result != LIBAB_SUCCESS) {
        eval_config_remove(&ab->lexer.config, op, token_type);
        free(new_entry);
    }

    return result;
}

libab_result libab_register_operator_infix(libab* ab, const char* op, int precedence, libab_function_ptr func) {
    return _register_operator(ab, op, TOKEN_OP_INFIX, precedence, func);
}

libab_result libab_register_operator_prefix(libab* ab, const char* op, libab_function_ptr func) {
    return _register_operator(ab, op, TOKEN_OP_PREFIX, 0, func);
}

libab_result libab_register_operator_postfix(libab* ab, const char* op, libab_function_ptr func) {
    return _register_operator(ab, op, TOKEN_OP_POSTFIX, 0, func);
}

libab_result libab_register_function(libab* ab, const char* name, libab_function_ptr func) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    if((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_FUN;
        new_entry->data_u.function.function = func;
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_table_put(&ab->table, name, new_entry);
    }

    if(result != LIBAB_SUCCESS) {
        free(new_entry);
    }

    return result;
}

libab_result libab_free(libab* ab) {
    libab_table_free(&ab->table);
    libab_parser_free(&ab->parser);
    return libab_lexer_free(&ab->lexer);
}
