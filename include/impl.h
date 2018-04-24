#ifndef LIBABACUS_IMPL_H
#define LIBABACUS_IMPL_H

/**
 * Implementation functions for things like numbers.
 */
struct libab_impl_s {
    /**
     * Function to parse a number from a string.
     */
    void* (*parse_num)(const char*);
};

typedef struct libab_impl_s libab_impl;

#endif
