#ifndef LIBABACUS_H
#define LIBABACUS_H

/**
 * An enum that represents the outcomes of
 * libabacus functions that can fail.
 */
enum libab_result {
    LIBAB_SUCCESS,
    LIBAB_MALLOC,
    LIBAB_BAD_PATTERN,
    LIBAB_FAILED_MATCH,
    LIBAB_EOF,
    LIBAB_UNEXPECTED
};

typedef enum libab_result libab_result;

#endif
