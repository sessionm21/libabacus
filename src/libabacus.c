#include "libabacus.h"
#include "debug.h"
#include "lexer.h"
#include "reserved.h"
#include "util.h"
#include "value.h"
#include <stdlib.h>
#include "free_functions.h"

static libab_basetype _basetype_function_list = {libab_free_function_list, NULL, 0};

static libab_basetype_param _basetype_function_params[] = {{BT_LIST, NULL}};

static libab_basetype _basetype_function = {libab_free_function,
                                            _basetype_function_params, 1};

static libab_basetype _basetype_unit = { libab_free_unit, NULL, 0 };

static libab_basetype _basetype_bool = { libab_free_bool, NULL, 0 };

libab_result _prepare_types(libab* ab, void (*free_function)(void*));

libab_result libab_init(libab* ab, void* (*parse_function)(const char*),
                        void (*free_function)(void*)) {
    int parser_initialized = 0;
    int lexer_initialized = 0;
    int interpreter_initialized = 0;
    libab_ref null_ref;
    libab_result result;
    libab_gc_list_init(&ab->containers);
    libab_ref_null(&null_ref);
    libab_ref_null(&ab->type_num);
    libab_ref_null(&ab->type_bool);
    libab_ref_null(&ab->type_function_list);
    libab_ref_null(&ab->type_unit);

    ab->impl.parse_num = parse_function;
    result = libab_create_table(ab, &ab->table, &null_ref);

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        libab_ref_free(&ab->type_bool);
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
        libab_ref_free(&ab->type_bool);
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
        result = libab_operator_init(new_operator, token_type, precedence, associativity,
                            function);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        libab_sanitize(op_buffer, op, 8);
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

libab_result _create_value_function_internal(libab* ab, 
                                             libab_ref* into, libab_ref* type,
                                             libab_function_ptr func, 
                                             libab_ref* scope) {
    libab_ref function_ref;
    libab_result result =
        libab_create_function_internal(ab, &function_ref, libab_free_function, func, scope);
    libab_ref_null(into);
    if (result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_create_value_ref(ab, into, &function_ref, type);
    }
    libab_ref_free(&function_ref);
    return result;
}

libab_result libab_register_function(libab* ab, const char* name,
                                     libab_ref* type, libab_function_ptr func) {
    libab_ref function_value;
    libab_result result =
        _create_value_function_internal(ab, &function_value, type, func, &ab->table);

    if (result == LIBAB_SUCCESS) {
        libab_overload_function(ab, libab_ref_get(&ab->table), name, &function_value);
    }
    libab_ref_free(&function_value);

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
    libab_ref_null(&ab->type_bool);
    libab_ref_null(&ab->type_function_list);
    libab_ref_null(&ab->type_unit);

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_num);
        result =
            libab_instantiate_basetype(&ab->basetype_num, &ab->type_num, 0);
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(&ab->type_bool);
        result =
            libab_instantiate_basetype(&_basetype_bool, &ab->type_bool, 0);
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

    if(result == LIBAB_SUCCESS) {
        result = libab_register_basetype(ab, "bool", &_basetype_bool);
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
        libab_ref_free(&ab->type_bool);
        libab_ref_free(&ab->type_function_list);
        libab_ref_free(&ab->type_unit);
        libab_ref_null(&ab->type_num);
        libab_ref_null(&ab->type_bool);
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
        result = libab_resolve_parsetype_inplace(libab_ref_get(into),
                                         libab_ref_get(&ab->table));
    }
    ll_foreach(&tokens, NULL, compare_always, libab_lexer_foreach_match_free);
    ll_free(&tokens);
    return result;
}

libab_basetype* libab_get_basetype_num(libab* ab) { return &ab->basetype_num; }

libab_basetype* libab_get_basetype_bool(libab* ab) { return &_basetype_bool; }

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

void libab_get_type_bool(libab* ab, libab_ref* into) {
    libab_ref_copy(&ab->type_bool, into);
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

void libab_get_true_value(libab* ab, libab_ref* into) {
    libab_interpreter_true_value(&ab->intr, into);
}

void libab_get_false_value(libab* ab, libab_ref* into) {
    libab_interpreter_false_value(&ab->intr, into);
}

void libab_get_bool_value(libab* ab, int val, libab_ref* into) {
    val ? libab_get_true_value(ab, into) : libab_get_false_value(ab, into);
}

libab_result libab_parse(libab* ab, const char* string, libab_tree** into) {
    libab_result result = LIBAB_SUCCESS;
    ll tokens;

    ll_init(&tokens);
    *into = NULL;
    result = libab_lexer_lex(&ab->lexer, string, &tokens);

    if(result == LIBAB_SUCCESS) {
        result = libab_parser_parse(&ab->parser, &tokens, string, into);
    }

    ll_foreach(&tokens, NULL, compare_always, libab_lexer_foreach_match_free);
    ll_free(&tokens);
    return result;
}

libab_result _handle_va_params(libab* ab, libab_ref_vec* into, size_t param_count, va_list args) {
    libab_result result = libab_ref_vec_init(into);
    if(result == LIBAB_SUCCESS) {
        while(result == LIBAB_SUCCESS && param_count--) {
            result = libab_ref_vec_insert(into, va_arg(args, libab_ref*));
        }

        if(result != LIBAB_SUCCESS) {
            libab_ref_vec_free(into);
        }
    }
    return result;
}

libab_result libab_run(libab* ab, const char* string, libab_ref* value) {
    libab_result result;
    libab_tree* root;

    libab_ref_null(value);
    result = libab_parse(ab, string, &root);

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(value);
        result = libab_interpreter_run(&ab->intr, root, &ab->table, SCOPE_FORCE, value);
        libab_tree_free_recursive(root);
    }

    return result;
}

libab_result libab_call_function(libab* ab, const char* function, 
                                libab_ref* into,
                                size_t param_count, ...) {
    libab_ref_vec params;
    va_list args;
    libab_result result = LIBAB_SUCCESS;

    va_start(args, param_count);
    libab_ref_null(into);
    result = _handle_va_params(ab, &params, param_count, args);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_interpreter_call_function(&ab->intr, &ab->table, function, &params, into);

        libab_ref_vec_free(&params);
    }
    va_end(args);

    return result;
}

libab_result libab_run_tree(libab* ab, libab_tree* tree, libab_ref* value) {
    return libab_interpreter_run(&ab->intr, tree, &ab->table, SCOPE_FORCE, value);
}

libab_result libab_run_scoped(libab* ab, const char* string, libab_ref* scope, libab_ref* into) {
    libab_result result;
    libab_tree* root;

    libab_ref_null(into);
    result = libab_parse(ab, string, &root);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_interpreter_run(&ab->intr, root, scope, SCOPE_NONE, into);
        libab_tree_free_recursive(root);
    }

    return result;
}

libab_result libab_call_function_scoped(libab* ab, const char* function, libab_ref* scope, libab_ref* into,
        size_t param_count, ...) {
    libab_ref_vec params;
    libab_result result;
    va_list args;

    va_start(args, param_count);
    libab_ref_null(into);
    result = _handle_va_params(ab, &params, param_count, args);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_interpreter_call_function(&ab->intr, scope, function, &params, into);

        libab_ref_vec_free(&params);
    }
    va_end(args);

    return result;
}

libab_result libab_run_tree_scoped(libab* ab, libab_tree* tree, libab_ref* scope, libab_ref* into) {
    return libab_interpreter_run(&ab->intr, tree, scope, SCOPE_NONE, into);
}

libab_result libab_free(libab* ab) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_free(libab_ref_get(&ab->table));
    libab_ref_free(&ab->table);
    libab_ref_free(&ab->type_num);
    libab_ref_free(&ab->type_bool);
    libab_ref_free(&ab->type_function_list);
    libab_ref_free(&ab->type_unit);
    libab_parser_free(&ab->parser);
    libab_interpreter_free(&ab->intr);
    result = libab_lexer_free(&ab->lexer);
    libab_gc_run(&ab->containers);
    return result;
}
