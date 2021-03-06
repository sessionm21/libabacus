#include "reserved.h"
#include "string.h"
#include "util.h"
#include "value.h"
#include "libabacus.h"

libab_result _behavior_assign(libab* ab, libab_ref* scope, 
                              libab_tree* left, libab_tree* right,
                              libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;

    if(left->variant == TREE_ID) {
        result = libab_run_tree_scoped(ab, right, scope, into);
        if(result == LIBAB_SUCCESS) {
            result = libab_set_variable(libab_ref_get(scope), left->string_value, into);
        }

        if(result != LIBAB_SUCCESS) {
            libab_ref_free(into);
        }
    } else {
        result = LIBAB_UNEXPECTED;
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

libab_result _behavior_method(libab* ab, libab_ref* scope,
                              libab_tree* left, libab_tree* right,
                              libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref param;
    libab_ref callee;
    libab_ref_vec params;
    int free_params = 0;
    size_t index = 0;

    libab_ref_null(&callee);
    if(right->variant == TREE_CALL) {
        result = libab_ref_vec_init(&params);
    } else {
        result = LIBAB_BAD_CALL;
    }

    if(result == LIBAB_SUCCESS) {
        free_params = 1;
        result = libab_run_tree_scoped(ab, left, scope, &param);
        if(result == LIBAB_SUCCESS) {
            result = libab_ref_vec_insert(&params, &param);
        }
        libab_ref_free(&param);
    }

    for(; index < right->children.size - 1 && result == LIBAB_SUCCESS; index++) {
        result = libab_run_tree_scoped(ab, vec_index(&right->children, index),
                                       scope, &param);
        if(result == LIBAB_SUCCESS) {
            result = libab_ref_vec_insert(&params, &param);
        }
        libab_ref_free(&param);
    }

    if(result == LIBAB_SUCCESS) {
        libab_ref_free(&callee);
        result = libab_run_tree_scoped(ab, vec_index(&right->children, right->children.size - 1),
                                      scope, &callee);
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_interpreter_call_value(&ab->intr, scope, &callee, &params, into);
    } else {
        libab_ref_null(into);
    }

    if(free_params) {
        libab_ref_vec_free(&params);
    }
    libab_ref_free(&callee);

    return result;
}

libab_result _expect_boolean(libab* ab, libab_ref* scope,
                          libab_tree* to_run, int* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref output;
    libab_value* value;
    libab_parsetype* type;

    result = libab_run_tree_scoped(ab, to_run, scope, &output);
    if(result == LIBAB_SUCCESS) {
        value = libab_ref_get(&output);
        type = libab_ref_get(&value->type);
        if(type->data_u.base != libab_get_basetype_bool(ab)) {
            result = LIBAB_BAD_CALL;
        } else {
            *into = *((int*) libab_ref_get(&value->data));
        }
    }
    libab_ref_free(&output);
    return result;
}

libab_result _behavior_land(libab* ab, libab_ref* scope,
                            libab_tree* left, libab_tree* right,
                            libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    int temp;
    result = _expect_boolean(ab, scope, left, &temp);
    if(result == LIBAB_SUCCESS && temp) {
        result = _expect_boolean(ab, scope, right, &temp);
    }

    if(result == LIBAB_SUCCESS) {
        libab_get_bool_value(ab, temp, into);
    } else {
        libab_ref_null(into);
    }

    return result;
}

libab_result _behavior_lor(libab* ab, libab_ref* scope,
                           libab_tree* left, libab_tree* right,
                           libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    int temp = 0;
    result = _expect_boolean(ab, scope, left, &temp);
    if(result == LIBAB_SUCCESS && !temp) {
        result = _expect_boolean(ab, scope, right, &temp);
    }

    if(result == LIBAB_SUCCESS) {
        libab_get_bool_value(ab, temp, into);
    } else {
        libab_ref_null(into);
    }

    return result;
}

static const libab_reserved_operator libab_reserved_operators[] = {
    {
        "=", /* Assignment */
        -3,  /* Lowest precedence */
        1,   /* Right associative, a = b = 6 should be a = (b = 6) */
        _behavior_assign
    },
    {
        ".",
        -2,
        -1,
        _behavior_method
    },
    {
        "&&", /* Logical and */
        -1,   /* Low precedence */
        -1,   /* Left associative. */
        _behavior_land
    },
    {
        "||", /* Logical or */
        -1,   /* Low precedence */
        -1,   /* Left associative. */
        _behavior_lor
    },
};

static const size_t element_count =
    sizeof(libab_reserved_operators) / sizeof(libab_reserved_operator);

const libab_reserved_operator* libab_find_reserved_operator(const char* name) {
    size_t i;
    for (i = 0; i < element_count; i++) {
        if (strcmp(name, libab_reserved_operators[i].op) == 0)
            return &libab_reserved_operators[i];
    }
    return NULL;
}

libab_result libab_register_reserved_operators(libab_lexer* lexer) {
    libab_result result = LIBAB_SUCCESS;
    char buffer[16];
    size_t i;
    for (i = 0; i < element_count && result == LIBAB_SUCCESS; i++) {
        libab_sanitize(buffer, libab_reserved_operators[i].op, 16);
        result = libab_convert_lex_result(eval_config_add(
            &lexer->config, buffer, TOKEN_OP_RESERVED));
    }
    return result;
}

libab_result libab_remove_reserved_operators(libab_lexer* lexer) {
    libab_result result = LIBAB_SUCCESS;
    char buffer[16];
    size_t i;
    for (i = 0; i < element_count && result == LIBAB_SUCCESS; i++) {
        libab_sanitize(buffer, libab_reserved_operators[i].op, 16);
        result = libab_convert_lex_result(eval_config_remove(
            &lexer->config, buffer, TOKEN_OP_RESERVED));
    }
    return result;
}
