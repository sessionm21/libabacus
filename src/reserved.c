#include "reserved.h"
#include "string.h"
#include "util.h"

libab_result _behavior_assign(libab* ab, libab_ref* scope, 
                              libab_tree* left, libab_tree* right,
                              libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;

    if(left->variant == TREE_ID) {
        result = libab_run_tree_scoped(ab, right, scope, into);
        if(result == LIBAB_SUCCESS) {
            result = libab_put_table_value(libab_ref_get(scope), left->string_value, into);
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

static const libab_reserved_operator libab_reserved_operators[] = {{
    "=", /* Assignment */
    0,   /* Lowest precedence */
    1,   /* Right associative, a = b = 6 should be a = (b = 6) */
    _behavior_assign
}};

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
    size_t i;
    for (i = 0; i < element_count && result == LIBAB_SUCCESS; i++) {
        result = libab_convert_lex_result(eval_config_add(
            &lexer->config, libab_reserved_operators[i].op, TOKEN_OP_RESERVED));
    }
    return result;
}

libab_result libab_remove_reserved_operators(libab_lexer* lexer) {
    libab_result result = LIBAB_SUCCESS;
    size_t i;
    for (i = 0; i < element_count && result == LIBAB_SUCCESS; i++) {
        result = libab_convert_lex_result(eval_config_remove(
            &lexer->config, libab_reserved_operators[i].op, TOKEN_OP_RESERVED));
    }
    return result;
}
