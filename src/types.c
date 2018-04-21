#include "types.h"

libab_result libab_array_init(libab_array* array) {
    return libab_ref_vec_init(&array->elems);
}

libab_result libab_array_insert(libab_array* array, libab_ref* value) {
    return libab_ref_vec_insert(&array->elems, value);
}

void libab_array_free(libab_array* array) { libab_ref_vec_free(&array->elems); }
