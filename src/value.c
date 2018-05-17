#include "value.h"
#include "parsetype.h"

void libab_value_init_ref(libab_value* value, libab_ref* data,
                          libab_ref* type) {
    libab_ref_copy(data, &value->data);
    libab_ref_copy(type, &value->type);
}

libab_result libab_value_init_raw(libab_value* value, void* data,
                                  libab_ref* type) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref tmp_ref;

    result = libab_ref_new(
        &tmp_ref, data,
        ((libab_parsetype*)libab_ref_get(type))->data_u.base->free_function);

    if (result == LIBAB_SUCCESS) {
        libab_value_init_ref(value, &tmp_ref, type);
        libab_ref_free(&tmp_ref);
    }

    return result;
}

void libab_value_free(libab_value* value) {
    libab_ref_free(&value->data);
    libab_ref_free(&value->type);
}
