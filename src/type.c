#include "type.h"
#include "util.h"
#include <stdlib.h>

void _libab_type_free(void* type) {
    free(((libab_type*) type)->name);
}

libab_result libab_type_create(libab_ref* ref, const char* name) {
    libab_type* type;
    libab_result result = LIBAB_SUCCESS;
    if((type = malloc(sizeof(*type)))) {
        result = libab_copy_string(&type->name, name);
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_ref_new(ref, type, _libab_type_free);
        if(result != LIBAB_SUCCESS) free(type->name);
    }

    if(result != LIBAB_SUCCESS) {
        free(type);
    }

    return result;
}
