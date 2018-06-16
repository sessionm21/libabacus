#include "libabacus.h"
#include "debug.h"
#include "lexer.h"
#include "reserved.h"
#include "util.h"
#include "value.h"
#include <stdlib.h>
#include "free_functions.h"

static libab_basetype _basetype_function_list = {free_function_list, NULL, 0};

static libab_basetype_param _basetype_function_params[] = {{BT_LIST, NULL}};

static libab_basetype _basetype_function = {free_function,
                                            _basetype_function_params, 1};

static libab_basetype _basetype_unit = { free_unit, NULL, 0 };

libab_result _prepare_types(libab* ab, void (*free_function)(void*));

libab_result libab_init(libab* ab, void* (*parse_function)(const char*),
                        void (*free_function)(void*)) {
    int parser_initialized = 0;
    int lexer_initialized = 0;
    int interpreter_initialized = 0;
    libab_ref null_ref;
    libab_result result;
    libab_ref_null(&null_ref);
    libab_ref_null(&ab->type_num);
    libab_ref_null(&ab->type_function_list);
    libab_ref_null(&ab->type_unit);

    ab->impl.parse_num = parse_function;
    result = libab_create_table(&ab->table, &null_ref);

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        libab_ref_free(&ab->type_function_list);
        libab_ref_free(&ab->type_unit);
        result = _prepare_types(ab, free_function);
    }

    if (result == LIBAB_SUCCESS) {
        parser_initialized = 1;
        libab_parser_init(&ab->parser, ab);
        result = libab_interpreter_init(&ab->intr, ab);
    }

    if(result == LIBAB_SUCCESS) {
        interpreter_initialized = 1;
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
        libab_ref_free(&ab->type_unit);

        if (parser_initialized) {
            libab_parser_free(&ab->parser);
        }

        if (interpreter_initialized) {
            libab_interpreter_free(&ab->intr);
        }

        if (lexer_initialized) {
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
    behavior->variant = BIMPL_INTERNAL;
    behavior->data_u.internal = func;
}

libab_result _register_operator(libab* ab, const char* op,
                                libab_operator_variant token_type,
                                int precedence, int associativity,
                                const char* function) {
    char op_buffer[8];
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* new_entry;
    libab_operator* new_operator = NULL;
    if ((new_entry = malloc(sizeof(*new_entry)))) {
        new_entry->variant = ENTRY_OP;
        new_operator = &(new_entry->data_u.op);
        libab_operator_init(new_operator, token_type, precedence, associativity,
                            function);
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
                                           const char* function) {
    return _register_operator(ab, op, OPERATOR_INFIX, precedence, associativity,
                              function);
}

libab_result libab_register_operator_prefix(libab* ab, const char* op,
                                            const char* function) {
    return _register_operator(ab, op, OPERATOR_PREFIX, 0, 0, function);
}

libab_result libab_register_operator_postfix(libab* ab, const char* op,
                                             const char* function) {
    return _register_operator(ab, op, OPERATOR_POSTFIX, 0, 0, function);
}

libab_result _create_value_function_internal(libab_ref* into, libab_ref* type,
                                             libab_function_ptr func, 
                                             libab_ref* scope) {
    libab_ref function_ref;
    libab_result result =
        libab_create_function_internal(&function_ref, free_function, func, scope);
    libab_ref_null(into);
    if (result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_create_value_ref(into, &function_ref, type);
    }
    libab_ref_free(&function_ref);
    return result;
}

libab_result _create_value_function_list(libab_ref* into, libab_ref* type) {
    libab_ref list_ref;
    libab_result result = libab_create_function_list(&list_ref, type);
    libab_ref_null(into);
    if (result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_create_value_ref(into, &list_ref, type);
    }
    libab_ref_free(&list_ref);
    return result;
}

libab_result _libab_register_function_existing(libab* ab,
                                               libab_table_entry* entry,
                                               libab_ref* function_val) {
    libab_value* old_value;
    libab_parsetype* old_type;
    libab_result result = LIBAB_SUCCESS;

    old_value = libab_ref_get(&entry->data_u.value);
    old_type = libab_ref_get(&old_value->type);

    if (old_type->data_u.base == &_basetype_function_list) {
        libab_function_list* list = libab_ref_get(&old_value->data);
        result = libab_function_list_insert(list, function_val);
    } else if (old_type->data_u.base == &_basetype_function) {
        libab_ref new_list;
        result =
            _create_value_function_list(&new_list, &ab->type_function_list);
        if (result == LIBAB_SUCCESS) {
            libab_function_list* list =
                libab_ref_get(&((libab_value*)libab_ref_get(&new_list))->data);
            result = libab_function_list_insert(list, &entry->data_u.value);
            if (result == LIBAB_SUCCESS) {
                result = libab_function_list_insert(list, function_val);
            }
        }
        if (result == LIBAB_SUCCESS) {
            libab_ref_swap(&entry->data_u.value, &new_list);
        }
        libab_ref_free(&new_list);
    } else {
        libab_ref_swap(&entry->data_u.value, function_val);
    }

    return result;
}

libab_result _libab_register_function_new(libab* ab, const char* name,
                                          libab_ref* function_val) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* entry;
    if ((entry = malloc(sizeof(*entry)))) {
        entry->variant = ENTRY_VALUE;
        libab_ref_copy(function_val, &entry->data_u.value);
        result = libab_table_put(libab_ref_get(&ab->table), name, entry);

        if (result != LIBAB_SUCCESS) {
            libab_table_entry_free(entry);
            free(entry);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    return result;
}

libab_result libab_register_function(libab* ab, const char* name,
                                     libab_ref* type, libab_function_ptr func) {
    libab_table_entry* existing_entry;
    libab_ref function_value;
    libab_result result =
        _create_value_function_internal(&function_value, type, func, &ab->table);

    if (result == LIBAB_SUCCESS) {
        existing_entry = libab_table_search_filter(
            libab_ref_get(&ab->table), name, NULL, libab_table_compare_value);
        if (existing_entry) {
            result = _libab_register_function_existing(ab, existing_entry,
                                                       &function_value);
        } else {
            result = _libab_register_function_new(ab, name, &function_value);
        }
        libab_ref_free(&function_value);
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

libab_result _prepare_types(libab* ab, void (*free_function)(void*)) {
    libab_result result = LIBAB_SUCCESS;

    ab->basetype_num.count = 0;
    ab->basetype_num.params = NULL;
    ab->basetype_num.free_function = free_function;

    libab_ref_null(&ab->type_num);
    libab_ref_null(&ab->type_function_list);
    libab_ref_null(&ab->type_unit);

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        result =
            libab_instantiate_basetype(&ab->basetype_num, &ab->type_num, 0);
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_function_list);
        result = libab_instantiate_basetype(&_basetype_function_list,
                                            &ab->type_function_list, 0);
    }

    if(result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_unit);
        result = libab_instantiate_basetype(&_basetype_unit, &ab->type_unit, 0);
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

    if (result == LIBAB_SUCCESS) {
        result = libab_register_basetype(ab, "unit", &_basetype_unit);
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        libab_ref_free(&ab->type_function_list);
        libab_ref_free(&ab->type_unit);
        libab_ref_null(&ab->type_num);
        libab_ref_null(&ab->type_function_list);
        libab_ref_null(&ab->type_unit);
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
    if (result == LIBAB_SUCCESS) {
        result = libab_resolve_parsetype(libab_ref_get(into),
                                         libab_ref_get(&ab->table));
    }
    ll_foreach(&tokens, NULL, compare_always, libab_lexer_foreach_match_free);
    ll_free(&tokens);
    return result;
}

libab_basetype* libab_get_basetype_num(libab* ab) { return &ab->basetype_num; }

libab_basetype* libab_get_basetype_function(libab* ab) {
    return &_basetype_function;
}

libab_basetype* libab_get_basetype_function_list(libab* ab) {
    return &_basetype_function_list;
}

libab_basetype* libab_get_basetype_unit(libab* ab) {
    return &_basetype_unit;
}

void libab_get_type_num(libab* ab, libab_ref* into) {
    libab_ref_copy(&ab->type_num, into);
}

void libab_get_type_function_list(libab* ab, libab_ref* into) {
    libab_ref_copy(&ab->type_function_list, into);
}

void libab_get_type_unit(libab* ab, libab_ref* into) {
    libab_ref_copy(&ab->type_unit, into);
}

void libab_get_unit_value(libab* ab, libab_ref* into) {
    libab_interpreter_unit_value(&ab->intr, into);
}

libab_result libab_run(libab* ab, const char* string, libab_ref* value) {
    libab_result result = LIBAB_SUCCESS;
    ll tokens;
    libab_tree* root;

    ll_init(&tokens);
    libab_ref_null(value);

    result = libab_lexer_lex(&ab->lexer, string, &tokens);

    if (result == LIBAB_SUCCESS) {
        result = libab_parser_parse(&ab->parser, &tokens, string, &root);
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(value);
        result = libab_interpreter_run(&ab->intr, root, value);
        libab_tree_free_recursive(root);
    }

    ll_foreach(&tokens, NULL, compare_always, libab_lexer_foreach_match_free);
    ll_free(&tokens);

    return result;
}

libab_result libab_run_function(libab* ab, const char* function, 
                                libab_ref* into,
                                size_t param_count, ...) {
    libab_ref_vec params;
    va_list args;
    libab_result result = LIBAB_SUCCESS;

    va_start(args, param_count);
    libab_ref_null(into);
    result = libab_ref_vec_init(&params);
    if(result == LIBAB_SUCCESS) {
        while(result == LIBAB_SUCCESS && param_count--) {
            result = libab_ref_vec_insert(&params, va_arg(args, libab_ref*));
        }

        if(result == LIBAB_SUCCESS) {
            libab_ref_free(into);
            result = libab_interpreter_run_function(&ab->intr, function, &params, into);
        }

        libab_ref_vec_free(&params);
    }
    va_end(args);

    return result;
}

libab_result libab_free(libab* ab) {
    libab_table_free(libab_ref_get(&ab->table));
    libab_ref_free(&ab->table);
    libab_ref_free(&ab->type_num);
    libab_ref_free(&ab->type_function_list);
    libab_ref_free(&ab->type_unit);
    libab_parser_free(&ab->parser);
    libab_interpreter_free(&ab->intr);
    return libab_lexer_free(&ab->lexer);
}
