#include "util.h"
#include <stdlib.h>

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
libab_result libab_copy_string_range(char** destination, const char* source, size_t from, size_t to) {
    libab_result result = LIBAB_SUCCESS;
    size_t string_length = to - from;
    if((*destination = malloc(string_length + 1)) == NULL) {
        result = LIBAB_MALLOC;
    } else {
        strncpy(*destination, source + from, string_length);
        (*destination)[string_length] = '\0';
    }
    return result;
}
libab_result libab_copy_string_size(char** destination, const char* source, size_t length) {
    return libab_copy_string_range(destination, source, 0, length);
}
libab_result libab_copy_string(char** destination, const char* source) {
    return libab_copy_string_range(destination, source, 0, strlen(source));
}
