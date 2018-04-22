#ifndef LIBABACUS_NUMBER_H
#define LIBABACUS_NUMBER_H

typedef void (*libab_num_function)(void*, void*, void*);
typedef void (*libab_num_function_unary)(void*, void*);

/**
 * Struct that holds information
 * about the implementation of numbers
 * that is being used by libab. This is not
 * assumed or inferred, and it's necessary to
 * provide all of these definitions.
 */
struct libab_number_impl_s {
    /**
     * The function used to allocate a number.
     */
    void* (*allocate)();
    /**
     * The function used to free a number.
     */
    void (*free)(void*);
    /**
     * The function used to construct a number
     * out of a string.
     */
    void* (*parse)(const char*);
    /**
     * A function to copy the value of one number
     * into the value of another.
     */
    void (*copy)(void*, void*);
};

typedef struct libab_number_impl_s libab_number_impl;

#endif
