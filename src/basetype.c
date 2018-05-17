#include "basetype.h"
#include "util.h"
#include <stdlib.h>

void libab_basetype_init(libab_basetype* basetype, void (*free_function)(void*),
                         int n, const libab_basetype_param params[]) {
    basetype->params = params;
    basetype->count = n;
    basetype->free_function = free_function;
}
void libab_basetype_free(libab_basetype* basetype) {}
