#include "libabacus_util.h"

libab_result libab_convert_lex_result(liblex_result to_convert) {
    libab_result result = LIBAB_SUCCESS;
    if(to_convert == LIBLEX_MALLOC) {
        result = LIBAB_MALLOC;
    } else if(to_convert == LIBLEX_INVALID) {
        result = LIBAB_BAD_PATTERN;
    } else if(to_convert == LIBLEX_UNRECOGNIZED) {
        result = LIBAB_FAILED_MATCH;
    }
    return result;
}
libab_result libab_convert_ds_result(libds_result to_convert) {
    libab_result result = LIBAB_SUCCESS;
    if(to_convert == LIBDS_MALLOC) {
        result = LIBAB_MALLOC;
    }
    return result;
}
