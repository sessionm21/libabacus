#ifndef LIBABACUS_FUNCTION_LIST_H
#define LIBABACUS_FUNCTION_LIST_H

#include "ref_vec.h"

struct libab_function_list_s {
    libab_ref_vec functions;
};

typedef struct libab_function_list_s libab_function_list;

libab_result libab_function_list_init(libab_function_list* list);
libab_result libab_function_list_insert(libab_function_list* list,
                                        libab_ref* function_value);
void libab_function_list_free(libab_function_list* list);

#endif
