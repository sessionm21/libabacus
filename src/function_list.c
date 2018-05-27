#include "function_list.h"

libab_result libab_function_list_init(libab_function_list* list) {
    return libab_ref_vec_init(&list->functions);
}

libab_result libab_function_list_insert(libab_function_list* list,
                                        libab_ref* function) {
    return libab_ref_vec_insert(&list->functions, function);
}

size_t libab_function_list_size(libab_function_list* list) {
    return list->functions.size;
}

void libab_function_list_index(libab_function_list* list, size_t index,
                               libab_ref* into) {
    libab_ref_vec_index(&list->functions, index, into);
}

void libab_function_list_free(libab_function_list* list) {
    libab_ref_vec_free(&list->functions);
}
