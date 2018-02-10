#ifndef LIBABACUS_H
#define LIBABACUS_H

/**
 * An enum that represents the outcomes of
 * libabacus functions that can fail.
 */
enum libab_result {
    LIBAB_SUCCESS,
    LIBAB_MALLOC
};

typedef enum libab_result libab_result;

#endif
