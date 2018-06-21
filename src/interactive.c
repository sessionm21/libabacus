#include "libabacus.h"
#include "util.h"
#include "value.h"
#include <stdio.h>
#include <math.h>

#define TRY(expression)                                                        \
    if (result == LIBAB_SUCCESS)                                               \
        result = expression;
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
        result = libab_create_value_raw(into, new_double, &type_num);
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

libab_result function_atan(libab* ab, libab_ref* scope, libab_ref_vec* params, libab_ref* into) {
    printf("atan called\n");
    double* val = libab_unwrap_param(params, 0);
    return create_double_value(ab, atan(*val), into);
}

libab_result function_atan2(libab* ab, libab_ref* scope, libab_ref_vec* params, libab_ref* into) {
    printf("atan2 called\n");
    double* left = libab_unwrap_param(params, 0);
    double* right = libab_unwrap_param(params, 1);
    return create_double_value(ab, atan2(*left, *right), into);
}

libab_result function_print_num(libab* ab, libab_ref* scope, libab_ref_vec* params, libab_ref* into) {
    double* param = libab_unwrap_param(params, 0);
    printf("%f\n", *param);
    libab_get_unit_value(ab, into);
    return LIBAB_SUCCESS;
}

libab_result function_print_unit(libab* ab, libab_ref* scope, libab_ref_vec* params, libab_ref* into) {
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

    result = libab_create_type(ab, &trig_type, "(num)->num");
    TRY(libab_create_type(ab, &atan2_type, "(num, num)->num"));
    TRY(libab_create_type(ab, &difficult_type, "((num)->num)->num"));
    TRY(libab_create_type(ab, &print_num_type, "(num)->unit"));
    TRY(libab_create_type(ab, &print_unit_type, "(unit)->unit"));

    TRY(libab_register_function(ab, "atan", &trig_type, function_atan));
    TRY(libab_register_function(ab, "atan2", &atan2_type, function_atan2));
    TRY(libab_register_function(ab, "plus", &atan2_type, function_plus));
    TRY(libab_register_function(ab, "minus", &atan2_type, function_minus));
    TRY(libab_register_function(ab, "times", &atan2_type, function_times));
    TRY(libab_register_function(ab, "divide", &atan2_type, function_divide));
    TRY(libab_register_function(ab, "print", &print_num_type, function_print_num));
    TRY(libab_register_function(ab, "print", &print_unit_type, function_print_unit));
    TRY(libab_register_operator_infix(ab, "+", 0, -1, "plus"));
    TRY(libab_register_operator_infix(ab, "-", 0, -1, "minus"));
    TRY(libab_register_operator_infix(ab, "*", 1, -1, "times"));
    TRY(libab_register_operator_infix(ab, "/", 1, -1, "divide"));

    libab_ref_free(&trig_type);
    libab_ref_free(&atan2_type);
    libab_ref_free(&difficult_type);
    libab_ref_free(&print_num_type);
    libab_ref_free(&print_unit_type);

    return result;
}

int main() {
    char input_buffer[2048];
    int interaction_count = INTERACTIONS;
    libab_ref eval_into;
    libab_ref call_into;
    libab_result result;
    libab_result eval_result;
    libab ab;

    if (libab_init(&ab, impl_parse, impl_free) != LIBAB_SUCCESS) {
        fprintf(stderr, "Failed to initialize libab.\n");
        exit(1);
    }

    result = register_functions(&ab);
    while (interaction_count-- && result == LIBAB_SUCCESS) {
        printf("(%d) > ", INTERACTIONS - interaction_count);
        fgets(input_buffer, 2048, stdin);
        eval_result = libab_run(&ab, input_buffer, &eval_into);

        if (eval_result != LIBAB_SUCCESS) {
            printf("Invalid input (error code %d).\n", eval_result);
        } else {
            result = libab_run_function(&ab, "print", &call_into, 1, &eval_into);
            libab_ref_free(&call_into);
        }
        libab_ref_free(&eval_into);
    }

    libab_free(&ab);
}
