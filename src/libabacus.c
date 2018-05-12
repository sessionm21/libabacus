#include "libabacus.h"
#include "lexer.h"
#include "reserved.h"
#include "util.h"
#include <stdlib.h>

libab_result _prepare_types(libab* ab, void (*free_function)(void*));

libab_result libab_init(libab* ab, void* (*parse_function)(const char*),
                        void (*free_function)(void*)) {
    int parser_initialized = 0;
    int lexer_initialized = 0;
    libab_ref null_ref;
    libab_result result;
    libab_ref_null(&null_ref);
    libab_ref_null(&ab->type_num);
    libab_ref_null(&ab->type_function_list);

    ab->impl.parse_num = parse_function;
    result = libab_create_table(&ab->table, &null_ref);

    if (result == LIBAB_SUCCESS) {
        result = _prepare_types(ab, free_function);
    }

    if(result == LIBAB_SUCCESS) {
        parser_initialized = 1;
        libab_parser_init(&ab->parser, ab);
        libab_interpreter_init(&ab->intr, ab);
        result = libab_lexer_init(&ab->lexer);
    }

    if (result == LIBAB_SUCCESS) {
        lexer_initialized = 1;
        result = libab_register_reserved_operators(&ab->lexer);
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_free(&ab->table);
        libab_ref_free(&ab->type_num);
        libab_ref_free(&ab->type_function_list);

        if(parser_initialized) {
            libab_parser_free(&ab->parser);
            libab_interpreter_free(&ab->intr);
        }

        if(lexer_initialized) {
            libab_lexer_free(&ab->lexer);
        }
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

void _initialize_behavior(libab_behavior* behavior, libab_ref* type,
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
    libab_operator* new_operator = NULL;
    if ((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_OP;
        new_operator = &(new_entry->data_u.op);
        libab_operator_init(new_operator, token_type,
                            precedence, associativity, type, func);
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
        if (new_operator)
            libab_operator_free(new_operator);
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
    libab_function* new_function = NULL;
    libab_table_entry* new_entry;
    if ((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_FUN;
        new_function = &new_entry->data_u.function;
        result = libab_function_init_internal(new_function, type, func);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_table_put(libab_ref_get(&ab->table), name, new_entry);
    }

    if (result != LIBAB_SUCCESS) {
        if(new_function)
            libab_function_free(new_function);
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

void _free_function_list(void* function_list) {
    libab_function_list_free(function_list);
    free(function_list);
}

static libab_basetype _basetype_function_list = {
    _free_function_list,
    NULL,
    0
};

void _free_function(void* function) {
    libab_function_free(function);
    free(function);
}

static libab_basetype_param _basetype_function_params[] = {
    { BT_LIST, NULL }
};

static libab_basetype _basetype_function = {
    _free_function,
    _basetype_function_params,
    1
};

libab_result _prepare_types(libab* ab, void (*free_function)(void*)) {
    libab_result result = LIBAB_SUCCESS;

    ab->basetype_num.count = 0;
    ab->basetype_num.params = NULL;
    ab->basetype_num.free_function = free_function;

    libab_ref_null(&ab->type_num);
    libab_ref_null(&ab->type_function_list);

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        result = libab_instantiate_basetype(&ab->basetype_num, 
                                            &ab->type_num, 0);
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_function_list);
        result = libab_instantiate_basetype(&_basetype_function_list,
                                            &ab->type_function_list, 0);
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_register_basetype(ab, "num", &ab->basetype_num);
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_register_basetype(ab, "function", &_basetype_function);
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_register_basetype(ab, "function_list", 
                                         &_basetype_function_list);
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        libab_ref_free(&ab->type_function_list);
        libab_ref_null(&ab->type_num);
        libab_ref_null(&ab->type_function_list);
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

libab_basetype* libab_get_basetype_num(libab* ab) {
    return &ab->basetype_num;
}

libab_basetype* libab_get_basetype_function(libab* ab) {
    return &_basetype_function;
}

libab_basetype* libab_get_basetype_function_list(libab* ab) {
    return &_basetype_function_list;
}

libab_result libab_free(libab* ab) {
    libab_ref_free(&ab->table);
    libab_ref_free(&ab->type_num);
    libab_ref_free(&ab->type_function_list);
    libab_parser_free(&ab->parser);
    libab_interpreter_free(&ab->intr);
    return libab_lexer_free(&ab->lexer);
}
