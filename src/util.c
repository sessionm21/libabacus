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
libab_result _libab_check_parsetype(libab_parsetype* to_check) {
    libab_result result = LIBAB_SUCCESS;
    return result;
}
libab_result libab_resolve_parsetype(libab_parsetype* to_resolve, libab_table* scope) {
    libab_result result = LIBAB_SUCCESS;
    int resolve_name, check_parents;
    size_t index = 0;
    resolve_name = !(to_resolve->variant & (LIBABACUS_TYPE_F_RESOLVED | LIBABACUS_TYPE_F_PLACE));
    check_parents = !(to_resolve->variant & LIBABACUS_TYPE_F_PLACE);
    if(resolve_name) {
        libab_basetype* basetype = libab_table_search_basetype(scope, to_resolve->data_u.name);
        if(basetype) {
            free(to_resolve->data_u.name);
            to_resolve->data_u.base = basetype;
            to_resolve->variant |= LIBABACUS_TYPE_F_RESOLVED;
        } else {
            result = LIBAB_BAD_TYPE;
        }
    }

    if(check_parents && result == LIBAB_SUCCESS) {
        if(to_resolve->variant & LIBABACUS_TYPE_F_PARENT) {
            result = _libab_check_parsetype(to_resolve);
        } else if(to_resolve->data_u.base->count) {
            result = LIBAB_BAD_TYPE;
        }
    }

    if(to_resolve->variant & LIBABACUS_TYPE_F_PARENT) {
        while(result == LIBAB_SUCCESS && index < to_resolve->children.size) {
            result = libab_resolve_parsetype(libab_ref_get(&to_resolve->children.data[index]), scope);
            index++;
        }
    }

    return result;
}
