#ifndef LIBABACUS_REF_VEC_H
#define LIBABACUS_REF_VEC_H

#include "refcount.h"
#include "result.h"
#include <stdlib.h>

#define LIBABACUS_REF_VEC_INITIAL_SIZE 4

/**
 * A vector of reference counter pointers.
 */
struct libab_ref_vec_s {
    /*
     * The capacity of the vector (how many elements
     * can be stored inside).
     */
    size_t capacity;
    /**
     * The number of elements inside the vector.
     */
    size_t size;
    /**
     * The array of references.
     */
    libab_ref* data;
};

typedef struct libab_ref_vec_s libab_ref_vec;

/**
 * Initializes a new vector.
 * @param vec the vector to initialize.
 * @return the result of the initialization.
 */
libab_result libab_ref_vec_init(libab_ref_vec* vec);
/**
 * Initializes a new vector as a copy another.
 * @param vec the vector to initialize.
 * @param copy_of the vector to make a copy of.
 * @return the result of the initialization.
 */
libab_result libab_ref_vec_init_copy(libab_ref_vec* vec, libab_ref_vec* copy_of);
/**
 * Inserts an existing reference counted value into the vector.
 * This bumps the reference's refcount, thereby preventing its
 * deallocation.
 * @param vec the vector to insert into.
 * @param data the reference to insert.
 * @return the result of the insertion.
 */
libab_result libab_ref_vec_insert(libab_ref_vec* vec, libab_ref* data);
/**
 * Inserts an allocated value into the vector after wrapping it in a reference.
 * @param vec the vector to insert into.
 * @param data the value to convert to a refcounted pointer.
 * @param free_func the function called to release the value (besides free)
 * @return the result of the insertion.
 */
libab_result libab_ref_vec_insert_value(libab_ref_vec* vec, void* data,
                                        void (*free_func)(void*));
/**
 * Returns the value at the given index in the vector, or null or the value
 * doesn't exist.
 * @param vec the vector to get a value from.
 * @param index the index to look at.
 * @param into the reference to store the result value into.
 * @return the reference stored at the given index.
 */
void libab_ref_vec_index(libab_ref_vec* vec, size_t index, libab_ref* into);
/**
 * Clears the reference counted vector, without resizing.
 * @param vec the vector to clear.
 */
void libab_ref_vec_clear(libab_ref_vec* vec);
/**
 * Releases the memory allocated by the vector.
 * The references stored in the vector have their refcount decreased.
 * @param vec the vector to free.
 */
void libab_ref_vec_free(libab_ref_vec* vec);

#endif
