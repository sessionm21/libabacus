#include "parsetype.h"
#include <stdlib.h>

int _foreach_free_child(void* data, va_list args) {
    libab_parsetype_free_recursive(data);
    return 0;
}
void libab_parsetype_free(libab_parsetype* type) {
    if(!(type->variant & LIBABACUS_TYPE_F_RESOLVED)) {
        free(type->data_u.name);
    }
}
void libab_parsetype_free_recursive(libab_parsetype* type) {
    if(type->variant & LIBABACUS_TYPE_F_PARENT) {
        vec_foreach(&(type->children), NULL, compare_always, _foreach_free_child);
        vec_free(&(type->children));
    }
    libab_parsetype_free(type);
    free(type);
}
