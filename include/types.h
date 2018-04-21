#ifndef LIBABACUS_TYPES_H
#define LIBABACUS_TYPES_H

#include "ref_vec.h"

/**
 * A struct that represents an array
 * in libab.
 */
struct libab_array_s {
    /**
     * The elements in the array.
     */
    libab_ref_vec elems;
};

typedef struct libab_array_s libab_array;

/**
 * Initializes the array.
 * @param array the array to initialize.
 * @return the result of the initialization.
 */
libab_result libab_array_init(libab_array* array);
/**
 * Inserts an element into the array.
 * @param array the array to insert.
 * @param value the interpreter value to insert.
 * @return the result of the insertion.
 */
libab_result libab_array_insert(libab_array* array, libab_ref* value);
/**
 * Frees the given array.
 * @param array the array to free.
 */
void libab_array_free(libab_array* array);

#endif
