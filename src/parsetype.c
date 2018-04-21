#include "parsetype.h"
#include <stdlib.h>

void libab_parsetype_free(libab_parsetype* type) {
    if (!(type->variant & LIBABACUS_TYPE_F_RESOLVED)) {
        free(type->data_u.name);
    }
    if (type->variant & LIBABACUS_TYPE_F_PARENT) {
        libab_ref_vec_free(&(type->children));
    }
}
