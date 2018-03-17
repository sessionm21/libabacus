#include "reserved.h"
#include "string.h"
#include "util.h"

static const libab_reserved_operator libab_reserved_operators[] = {
    {
        "=", /* Assignment */
        0, /* Lowest precedence */
        1 /* Right associative, a = b = 6 should be a = (b = 6) */
    }
};
static const size_t element_count = 
    sizeof(libab_reserved_operators) / sizeof(libab_reserved_operator);

const libab_reserved_operator* libab_find_reserved_operator(const char* name) {
    size_t i;
    for(i = 0; i < element_count; i++) {
        if(strcmp(name, libab_reserved_operators[i].op) == 0)
            return &libab_reserved_operators[i];
    }
    return NULL;
}

libab_result libab_register_reserved_operators(libab_lexer* lexer) {
    libab_result result = LIBAB_SUCCESS;
    size_t i;
    for(i = 0; i < element_count && result == LIBAB_SUCCESS; i++) {
        result = libab_convert_lex_result(eval_config_add(&lexer->config,
                    libab_reserved_operators[i].op, TOKEN_OP_RESERVED));
    }
    return result;
}

libab_result libab_remove_reserved_operators(libab_lexer* lexer) {
    libab_result result = LIBAB_SUCCESS;
    size_t i;
    for(i = 0; i < element_count && result == LIBAB_SUCCESS; i++) {
        result = libab_convert_lex_result(eval_config_remove(&lexer->config,
                    libab_reserved_operators[i].op, TOKEN_OP_RESERVED));
    }
    return result;
}
