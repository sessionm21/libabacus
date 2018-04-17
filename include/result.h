#ifndef LIBABACUS_RESULT_H
#define LIBABACUS_RESULT_H

/**
 * An enum that represents the outcomes of
 * libabacus functions that can fail.
 */
enum libab_result_e {
    LIBAB_SUCCESS,
    LIBAB_MALLOC,
    LIBAB_BAD_PATTERN,
    LIBAB_FAILED_MATCH,
    LIBAB_UNKNOWN_TYPE,
    LIBAB_BAD_TYPE,
    LIBAB_EOF,
    LIBAB_UNEXPECTED
};

typedef enum libab_result_e libab_result;

#endif
