#include "function_list.h"

libab_result libab_function_list_init(libab_function_list* list) {
    return libab_ref_vec_init(&list->functions);
}

libab_result libab_function_list_insert(libab_function_list* list,
                                        libab_ref* function) {
    return libab_ref_vec_insert(&list->functions, function);
}

void libab_function_list_free(libab_function_list* list) {
    libab_ref_vec_free(&list->functions);
}
