#include "value.h"
#include "parsetype.h"

void libab_value_init(libab_value* value, void* data, libab_ref* type) {
    value->data = data;
    libab_ref_copy(type, &value->type);
}

void libab_value_free(libab_value* value) {
    void (*free_function)(void*);
    libab_parsetype* value_type;
    libab_ref_free(&value->type);
    value_type = libab_ref_get(&value->type);
    free_function = value_type->data_u.base->free_function;
    if(free_function) free_function(value->data);
}
