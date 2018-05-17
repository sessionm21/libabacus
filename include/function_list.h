#ifndef LIBABACUS_FUNCTION_LIST_H
#define LIBABACUS_FUNCTION_LIST_H

#include "ref_vec.h"

/**
 * A list of function values,
 * returned if a name of an overloaded
 * function is used.
 */
struct libab_function_list_s {
    /**
     * The function list.
     */
    libab_ref_vec functions;
};

typedef struct libab_function_list_s libab_function_list;

/**
 * Initializes a function list.
 * @param list the list to intialize.
 * @return the result of the initialization.
 */
libab_result libab_function_list_init(libab_function_list* list);
/**
 * Inserts a new function value into the list.
 * @param list the list to insert into.
 * @param function_value the function value to insert.
 * @return the result of the insertion.
 */
libab_result libab_function_list_insert(libab_function_list* list,
                                        libab_ref* function_value);
/**
 * Frees the given function list.
 * @param list the list to free.
 */
void libab_function_list_free(libab_function_list* list);

#endif
