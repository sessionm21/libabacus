#ifndef LIBABACUS_REFCOUNT_H
#define LIBABACUS_REFCOUNT_H

#include "result.h"
#include "gc_functions.h"

/**
 * A struct for holding
 * the number of references
 * to a value, as well as the function required
 * to free the value.
 */
struct libab_ref_count_s {
    /**
     * The value this reference holds.
     */
    void* data;
    /**
     * The fucntion to free the value.
     * Can be NULL for no-op.
     */
    void (*free_func)(void* data);
    /**
     * The number of references that
     * prevent the deallocation of the value.
     */
    int strong;
    /**
     * The number of references
     * that still exist, even to a freed instance.
     */
    int weak;
    /**
     * The number of outside references.
     * This is used for garbage collection.
     */
    int gc;
    /**
     * Previous pointer for garbage collection
     * linked list.
     */
    struct libab_ref_count_s* prev;
    /**
     * Next pointer for garbage collection
     * linked list.
     */
    struct libab_ref_count_s* next;
    /**
     * Function used to visit child containers,
     * used by GC.
     */
    libab_visit_function_ptr visit_children;
};

/**
 * A reference to a value.
 */
struct libab_ref_s {
    /**
     * Whether this reference is a strong reference.
     */
    unsigned int strong : 1;
    /**
     * Whether this reference is a NULL reference.
     */
    unsigned int null : 1;
    /**
     * The reference count struct keeping track
     * of how many references are pointing to the value.
     */
    struct libab_ref_count_s* count;
};

typedef struct libab_ref_s libab_ref;
typedef struct libab_ref_count_s libab_ref_count;

/**
 * Creates a new referene, using the given data and free function.
 * @param ref the reference to initialize with the given data.
 * @param data the data to reference count.
 * @param free_func the function to use to realease the data when refcount
 * reaches 0.
 * @return the result of the construction of the reference.
 */
libab_result libab_ref_new(libab_ref* ref, void* data,
                           void (*free_func)(void* data));
/**
 * Creates a reference to NULL. This does
 * not require a memory allocation.
 * @param ref the reference to initialize with null.
 */
void libab_ref_null(libab_ref* ref);
/**
 * Turns the given reference into a weak reference,
 * making it not keep the data allocated.
 */
void libab_ref_weaken(libab_ref* ref);
/**
 * Releases this particular reference to the data.
 * This doesn't necessarily free the underlying data.
 */
void libab_ref_free(libab_ref* ref);
/**
 * Copies this reference, thereby increasing the reference count.
 */
void libab_ref_copy(const libab_ref* ref, libab_ref* into);
/**
 * Swaps the contents of two references.
 */
void libab_ref_swap(libab_ref* left, libab_ref* right);
/**
 * Function that can be passed in to refcount to simply use free
 * when the refcount reaches 0.
 */
void libab_ref_data_free(void*);
/**
 * Gets the value of the reference.
 */
void* libab_ref_get(const libab_ref* ref);

#endif
