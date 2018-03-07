#include "parsetype.h"
#include <stdlib.h>

int _foreach_free_child(void* data, va_list args) {
    libab_parsetype_free_recursive(data);
    return 0;
}
void libab_parsetype_free(libab_parsetype* type) {
    free(type->name);
}
void libab_parsetype_free_recursive(libab_parsetype* type) {
    if(type->variant == PT_PARENT) {
        vec_foreach(&(type->children), NULL, compare_always, _foreach_free_child);
        vec_free(&(type->children));
    }
    libab_parsetype_free(type);
    free(type);
}
