#include "util.h"
#include "value.h"
#include <stdarg.h>
#include <stdlib.h>

libab_result libab_convert_lex_result(liblex_result to_convert) {
    libab_result result = LIBAB_SUCCESS;
    if (to_convert == LIBLEX_MALLOC) {
        result = LIBAB_MALLOC;
    } else if (to_convert == LIBLEX_INVALID) {
        result = LIBAB_BAD_PATTERN;
    } else if (to_convert == LIBLEX_UNRECOGNIZED) {
        result = LIBAB_FAILED_MATCH;
    }
    return result;
}
libab_result libab_convert_ds_result(libds_result to_convert) {
    libab_result result = LIBAB_SUCCESS;
    if (to_convert == LIBDS_MALLOC) {
        result = LIBAB_MALLOC;
    }
    return result;
}
libab_result libab_copy_string_range(char** destination, const char* source,
                                     size_t from, size_t to) {
    libab_result result = LIBAB_SUCCESS;
    size_t string_length = to - from;
    if ((*destination = malloc(string_length + 1)) == NULL) {
        result = LIBAB_MALLOC;
    } else {
        strncpy(*destination, source + from, string_length);
        (*destination)[string_length] = '\0';
    }
    return result;
}
libab_result libab_copy_string_size(char** destination, const char* source,
                                    size_t length) {
    return libab_copy_string_range(destination, source, 0, length);
}
libab_result libab_copy_string(char** destination, const char* source) {
    return libab_copy_string_range(destination, source, 0, strlen(source));
}
libab_result _libab_check_parsetype(libab_parsetype* to_check) {
    libab_result result = LIBAB_SUCCESS;
    return result;
}
libab_result libab_resolve_parsetype(libab_parsetype* to_resolve,
                                     libab_table* scope) {
    libab_result result = LIBAB_SUCCESS;
    int resolve_name, check_parents;
    size_t index = 0;
    resolve_name = !(to_resolve->variant &
                     (LIBABACUS_TYPE_F_RESOLVED | LIBABACUS_TYPE_F_PLACE));
    check_parents = !(to_resolve->variant & LIBABACUS_TYPE_F_PLACE);

    if ((to_resolve->variant & LIBABACUS_TYPE_F_PLACE) && (to_resolve->variant & LIBABACUS_TYPE_F_PARENT)) {
        result = LIBAB_UNEXPECTED;
    }

    if (resolve_name && result == LIBAB_SUCCESS) {
        libab_basetype* basetype =
            libab_table_search_basetype(scope, to_resolve->data_u.name);
        if (basetype) {
            free(to_resolve->data_u.name);
            to_resolve->data_u.base = basetype;
            to_resolve->variant |= LIBABACUS_TYPE_F_RESOLVED;
        } else {
            result = LIBAB_UNKNOWN_TYPE;
        }
    }

    if (check_parents && result == LIBAB_SUCCESS) {
        if (to_resolve->variant & LIBABACUS_TYPE_F_PARENT) {
            result = _libab_check_parsetype(to_resolve);
        } else if (to_resolve->data_u.base->count) {
            result = LIBAB_BAD_TYPE;
        }
    }

    if (to_resolve->variant & LIBABACUS_TYPE_F_PARENT) {
        while (result == LIBAB_SUCCESS && index < to_resolve->children.size) {
            result = libab_resolve_parsetype(
                libab_ref_get(&to_resolve->children.data[index]), scope);
            index++;
        }
    }

    return result;
}

void _libab_free_parsetype(void* parsetype) {
    libab_parsetype_free(parsetype);
    free(parsetype);
}

void _libab_parsetype_free(void* parsetype) {
    libab_parsetype_free(parsetype);
    free(parsetype);
}

libab_result libab_instantiate_basetype(libab_basetype* to_instantiate,
                                        libab_ref* into, size_t n, ...) {
    libab_result result = LIBAB_SUCCESS;
    libab_parsetype* parsetype;
    va_list params;

    va_start(params, n);

    if ((parsetype = malloc(sizeof(*parsetype)))) {
        result = libab_parsetype_init_va(parsetype, to_instantiate, n, params);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, parsetype, _libab_parsetype_free);
        if (result != LIBAB_SUCCESS) {
            libab_parsetype_free(parsetype);
        }
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(parsetype);
    }

    va_end(params);

    return result;
}

void _free_table(void* data) {
    libab_table_free(data);
    free(data);
}

libab_result libab_create_table(libab_ref* into, libab_ref* parent) {
    libab_table* table;
    libab_result result = LIBAB_SUCCESS;
    if ((table = malloc(sizeof(*table)))) {
        libab_table_init(table);
        libab_table_set_parent(table, parent);
        result = libab_ref_new(into, table, _free_table);

        if (result != LIBAB_SUCCESS) {
            _free_table(table);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }
    return result;
}

void _free_value(void* value) {
    libab_value_free(value);
    free(value);
}

libab_result libab_create_value_ref(libab_ref* into, libab_ref* data,
                                    libab_ref* type) {
    libab_value* value;
    libab_result result = LIBAB_SUCCESS;
    if ((value = malloc(sizeof(*value)))) {
        libab_value_init_ref(value, data, type);
        result = libab_ref_new(into, value, _free_value);

        if (result != LIBAB_SUCCESS) {
            _free_value(value);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }
    return result;
}

libab_result libab_create_value_raw(libab_ref* into, void* data,
                                    libab_ref* type) {
    libab_value* value;
    libab_result result = LIBAB_SUCCESS;

    if ((value = malloc(sizeof(*value)))) {
        result = libab_value_init_raw(value, data, type);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, value, _free_value);
        if (result != LIBAB_SUCCESS) {
            libab_value_free(value);
        }
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(value);
    }

    return result;
}

libab_result libab_create_function_internal(libab_ref* into,
                                            void (*free_function)(void*),
                                            libab_function_ptr fun) {
    libab_function* new_function;
    libab_result result = LIBAB_SUCCESS;

    if ((new_function = malloc(sizeof(*new_function)))) {
        result = libab_function_init_internal(new_function, fun);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, new_function, free_function);
        if (result != LIBAB_SUCCESS) {
            libab_function_free(new_function);
        }
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(new_function);
    }

    return result;
}

libab_result libab_create_function_tree(libab_ref* into,
                                        void (*free_function)(void*),
                                        libab_tree* tree) {
    libab_function* new_function;
    libab_result result = LIBAB_SUCCESS;

    if ((new_function = malloc(sizeof(*new_function)))) {
        result = libab_function_init_tree(new_function, tree);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, new_function, free_function);
        if (result != LIBAB_SUCCESS) {
            libab_function_free(new_function);
        }
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(new_function);
    }

    return result;
}

libab_result libab_create_function_list(libab_ref* into, libab_ref* type) {
    libab_function_list* list;
    libab_result result = LIBAB_SUCCESS;

    if ((list = malloc(sizeof(*list)))) {
        result = libab_function_list_init(list);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, list,
                               ((libab_parsetype*)libab_ref_get(type))
                                   ->data_u.base->free_function);
        if (result != LIBAB_SUCCESS) {
            libab_function_list_free(list);
        }
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(list);
    }

    return result;
}

libab_result libab_put_table_value(libab_table* table, const char* key,
                                   libab_ref* value) {
    libab_table_entry* entry;
    libab_result result = LIBAB_SUCCESS;
    if ((entry = malloc(sizeof(*entry)))) {
        entry->variant = ENTRY_VALUE;
        libab_ref_copy(value, &entry->data_u.value);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_table_put(table, key, entry);
        if (result != LIBAB_SUCCESS) {
            libab_ref_free(&entry->data_u.value);
            free(entry);
        }
    }

    return result;
}

void* libab_unwrap_value(libab_ref* ref) {
    libab_value* value = libab_ref_get(ref);
    return libab_ref_get(&value->data);
}

void* libab_unwrap_param(libab_ref_vec* vec, size_t index) {
    libab_ref temp;
    void* data;
    libab_ref_vec_index(vec, index, &temp);
    data = libab_unwrap_value(&temp);
    libab_ref_free(&temp);
    return data;
}
