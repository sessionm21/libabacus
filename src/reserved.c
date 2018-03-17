#include "reserved.h"
#include "string.h"

static const libab_reserved_operator libab_reserved_operators[] = {
    {
        "=", /* Assignment */
        0, /* Lowest precedence */
        1 /* Right associative, a = b = 6 should be a = (b = 6) */
    }
};

const libab_reserved_operator* libab_find_reserved_operator(const char* name) {
    static const size_t element_count = 
        sizeof(libab_reserved_operators) / sizeof(libab_reserved_operator);
    size_t i;
    for(i = 0; i < element_count; i++) {
        if(strcmp(name, libab_reserved_operators[i].op) == 0)
            return &libab_reserved_operators[i];
    }
    return NULL;
}
