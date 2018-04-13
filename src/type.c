#include "type.h"

libab_result libab_type_init(libab_type* type, libab_basetype* base) {
    type->base = base;
    return libab_ref_vec_init(&type->params);
}

void libab_type_free(libab_type* type) {
    libab_ref_vec_free(&type->params);
}
