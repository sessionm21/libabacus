#include "libabacus.h"
#include "util.h"
#include "value.h"
#include <stdio.h>
#include <math.h>

#define TRY(expression)                                                        \
    if (result == LIBAB_SUCCESS)                                               \
        result = expression;
#define FUNCTION(name)                                                         \
    libab_result function_##name (libab* ab, libab_ref* scope,                 \
        libab_ref_vec* params, libab_ref* into)
    
#define INTERACTIONS 5

void* impl_parse(const char* string) {
    double* data = malloc(sizeof(*data));
    if (data) {
        *data = strtod(string, NULL);
    }
    return data;
}

void impl_free(void* data) { free(data); }

libab_result create_double_value(libab* ab, double val, libab_ref* into) {
    libab_ref type_num;
    libab_result result = LIBAB_SUCCESS;
    libab_get_type_num(ab, &type_num);
    double* new_double = malloc(sizeof(*new_double));
    if(new_double) {
        *new_double = val;
        result = libab_create_value_raw(ab, into, new_double, &type_num);
        if(result != LIBAB_SUCCESS) {
            free(new_double);
        }
    } else {
        result = LIBAB_MALLOC;
        libab_ref_null(into);
    }
    libab_ref_free(&type_num);
    return result;
}


FUNCTION(not) {
    int* left = libab_unwrap_param(params, 0);
    libab_get_bool_value(ab, !(*left), into);
    return LIBAB_SUCCESS;
}

FUNCTION(and) {
    int* left = libab_unwrap_param(params, 0);
    int* right = libab_unwrap_param(params, 1);
    libab_get_bool_value(ab, *left & *right, into);
    return LIBAB_SUCCESS;
}

FUNCTION(or) {
    int* left = libab_unwrap_param(params, 0);
    int* right = libab_unwrap_param(params, 1);
    libab_get_bool_value(ab, *left | *right, into);
    return LIBAB_SUCCESS;
}

FUNCTION(xor) {
    int* left = libab_unwrap_param(params, 0);
    int* right = libab_unwrap_param(params, 1);
    libab_get_bool_value(ab, *left ^ *right, into);
    return LIBAB_SUCCESS;
}

FUNCTION(atan) {
    printf("atan called\n");
    double* val = libab_unwrap_param(params, 0);
    return create_double_value(ab, atan(*val), into);
}

FUNCTION(atan2) {
    printf("atan2 called\n");
    double* left = libab_unwrap_param(params, 0);
    double* right = libab_unwrap_param(params, 1);
    return create_double_value(ab, atan2(*left, *right), into);
}

FUNCTION(equals_num) {
    double* left = libab_unwrap_param(params, 0);
    double* right = libab_unwrap_param(params, 1);
    libab_get_bool_value(ab, *left == *right, into);
    return LIBAB_SUCCESS;
}

FUNCTION(print_num) {
    double* param = libab_unwrap_param(params, 0);
    printf("%f\n", *param);
    libab_get_unit_value(ab, into);
    return LIBAB_SUCCESS;
}

FUNCTION(print_bool) {
    int* param = libab_unwrap_param(params, 0);
    printf("%s\n", *param ? "true" : "false");
    libab_get_unit_value(ab, into);
    return LIBAB_SUCCESS;
}

FUNCTION(print_unit) {
    printf("()\n");
    libab_get_unit_value(ab, into);
    return LIBAB_SUCCESS;
}

#define OP_FUNCTION(name, expression) \
    libab_result name(libab* ab, libab_ref* scope, libab_ref_vec* params, libab_ref* into) { \
        libab_result result = LIBAB_SUCCESS; \
        double right; \
        double left; \
        printf(#name " called\n"); \
        left = *((double*)libab_unwrap_param(params, 0)); \
        right = *((double*)libab_unwrap_param(params, 1)); \
        create_double_value(ab, expression, into); \
        return result;\
    }

OP_FUNCTION(function_plus, left + right)
OP_FUNCTION(function_minus, left - right)
OP_FUNCTION(function_times, left * right)
OP_FUNCTION(function_divide, left / right)

libab_result register_functions(libab* ab) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref trig_type;
    libab_ref atan2_type;
    libab_ref difficult_type;
    libab_ref print_num_type;
    libab_ref print_unit_type;
    libab_ref print_bool_type;
    libab_ref bool_logic_type;
    libab_ref bool_not_type;
    libab_ref equals_num_type;

    result = libab_create_type(ab, &trig_type, "(num)->num");
    TRY(libab_create_type(ab, &atan2_type, "(num, num)->num"));
    TRY(libab_create_type(ab, &equals_num_type, "(num, num)->bool"));
    TRY(libab_create_type(ab, &difficult_type, "((num)->num)->num"));
    TRY(libab_create_type(ab, &print_num_type, "(num)->unit"));
    TRY(libab_create_type(ab, &print_unit_type, "(unit)->unit"));
    TRY(libab_create_type(ab, &print_bool_type, "(bool)->unit"));
    TRY(libab_create_type(ab, &bool_logic_type, "(bool,bool)->bool"));
    TRY(libab_create_type(ab, &bool_not_type, "(bool)->bool"));

    TRY(libab_register_function(ab, "atan", &trig_type, function_atan));
    TRY(libab_register_function(ab, "atan2", &atan2_type, function_atan2));
    TRY(libab_register_function(ab, "plus", &atan2_type, function_plus));
    TRY(libab_register_function(ab, "minus", &atan2_type, function_minus));
    TRY(libab_register_function(ab, "times", &atan2_type, function_times));
    TRY(libab_register_function(ab, "divide", &atan2_type, function_divide));
    TRY(libab_register_function(ab, "and", &bool_logic_type, function_and));
    TRY(libab_register_function(ab, "or", &bool_logic_type, function_or));
    TRY(libab_register_function(ab, "xor", &bool_logic_type, function_xor));
    TRY(libab_register_function(ab, "not", &bool_not_type, function_not));
    TRY(libab_register_function(ab, "equals", &equals_num_type, function_equals_num));
    TRY(libab_register_function(ab, "print", &print_num_type, function_print_num));
    TRY(libab_register_function(ab, "print", &print_unit_type, function_print_unit));
    TRY(libab_register_function(ab, "print", &print_bool_type, function_print_bool));
    TRY(libab_register_operator_infix(ab, "==", 0, -1, "equals"));
    TRY(libab_register_operator_infix(ab, "+", 0, -1, "plus"));
    TRY(libab_register_operator_infix(ab, "-", 0, -1, "minus"));
    TRY(libab_register_operator_infix(ab, "*", 1, -1, "times"));
    TRY(libab_register_operator_infix(ab, "/", 1, -1, "divide"));
    TRY(libab_register_operator_infix(ab, "&", 1, -1, "and"));
    TRY(libab_register_operator_infix(ab, "|", 1, -1, "or"));
    TRY(libab_register_operator_infix(ab, "^", 1, -1, "xor"));
    TRY(libab_register_operator_prefix(ab, "!", "not"));

    libab_ref_free(&trig_type);
    libab_ref_free(&atan2_type);
    libab_ref_free(&difficult_type);
    libab_ref_free(&equals_num_type);
    libab_ref_free(&print_num_type);
    libab_ref_free(&print_unit_type);
    libab_ref_free(&print_bool_type);
    libab_ref_free(&bool_logic_type);
    libab_ref_free(&bool_not_type);

    return result;
}

libab_result loop(libab* ab, int interaction_count, libab_ref* scope) {
    libab_result result = LIBAB_SUCCESS;
    char input_buffer[2048];
    libab_ref eval_into;
    libab_ref call_into;
    libab_result eval_result;

    while (interaction_count-- && result == LIBAB_SUCCESS) {
        printf("(%d) > ", INTERACTIONS - interaction_count);
        fgets(input_buffer, 2048, stdin);
        eval_result = libab_run_scoped(ab, input_buffer, scope, &eval_into);

        if (eval_result != LIBAB_SUCCESS) {
            printf("Invalid input (error code %d).\n", eval_result);
        } else {
            result = libab_run_function_scoped(ab, "print", scope, &call_into, 1, &eval_into);
            if(result == LIBAB_BAD_CALL) {
                printf("(?)\n");
                result = LIBAB_SUCCESS;
            }
            libab_ref_free(&call_into);
        }
        libab_ref_free(&eval_into);
    }

    return result;
}

int main() {
    libab_result result;
    libab_ref scope;
    libab_ref test;
    libab ab;

    if (libab_init(&ab, impl_parse, impl_free) != LIBAB_SUCCESS) {
        fprintf(stderr, "Failed to initialize libab.\n");
        exit(1);
    }

    result = register_functions(&ab);
    if(result == LIBAB_SUCCESS) {
        result = libab_create_table(&ab, &scope, &ab.table);
    }
    if(result == LIBAB_SUCCESS) {
        loop(&ab, INTERACTIONS, &scope);
        libab_table_search_value(libab_ref_get(&scope), "test", &test);
        printf("%p\n", libab_ref_get(&test));
        libab_ref_free(&scope);
    }

    libab_free(&ab);
}
