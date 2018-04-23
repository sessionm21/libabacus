#include "parsetype.h"
#include <stdlib.h>

libab_result libab_parsetpe_init(libab_parsetype* type, libab_basetype* from,
        size_t n, ...) {
    libab_result result;
    va_list params;
    va_start(params, n);
    result = libab_parsetype_init_va(type, from, n, params);
    va_end(params);
    return result;
}

libab_result libab_parsetype_init_va(libab_parsetype* type, libab_basetype* from,
        size_t n, va_list args) {
    libab_result result = LIBAB_SUCCESS;
    size_t base_index = 0, param_index = 0;
    int free_vec = 0;

    if(from->count > n) result = LIBAB_BAD_TYPE;
    while(base_index < from->count && param_index < n) {
        if(from->params[base_index].variant == BT_NAME) {
            base_index++;
        }
        param_index++;
    }

    if(param_index < n) {
        result = LIBAB_BAD_TYPE;
    }

    type->data_u.base = from;
    type->variant = LIBABACUS_TYPE_F_RESOLVED; 
    if(result == LIBAB_SUCCESS && n) {
        type->variant |= LIBABACUS_TYPE_F_PARENT;
        result = libab_ref_vec_init(&type->children);
        if(result == LIBAB_SUCCESS) free_vec = 1;
    }

    while(n-- && result == LIBAB_SUCCESS) {
        libab_ref* ref = va_arg(args, libab_ref*);
        result = libab_ref_vec_insert(&type->children, ref);
    }

    if(free_vec) {
        libab_ref_vec_free(&type->children);
    }

    return result;
}

void libab_parsetype_free(libab_parsetype* type) {
    if (!(type->variant & LIBABACUS_TYPE_F_RESOLVED)) {
        free(type->data_u.name);
    }
    if (type->variant & LIBABACUS_TYPE_F_PARENT) {
        libab_ref_vec_free(&(type->children));
    }
}
