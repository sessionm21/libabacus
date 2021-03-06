#include "util.h"
#include "value.h"
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include "free_functions.h"

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
libab_result libab_resolve_parsetype_inplace(libab_parsetype* to_resolve,
                                     libab_table* scope) {
    libab_result result = LIBAB_SUCCESS;
    int resolve_name, check_parents;
    size_t index = 0;
    resolve_name = !(to_resolve->variant &
                     (LIBABACUS_TYPE_F_RESOLVED | LIBABACUS_TYPE_F_PLACE));
    check_parents = !(to_resolve->variant & LIBABACUS_TYPE_F_PLACE);

    if ((to_resolve->variant & LIBABACUS_TYPE_F_PLACE) &&
        (to_resolve->variant & LIBABACUS_TYPE_F_PARENT)) {
        result = LIBAB_UNEXPECTED;
    }

    if (resolve_name && result == LIBAB_SUCCESS) {
        libab_basetype* basetype =
            libab_get_basetype(to_resolve, scope);
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
            result = libab_resolve_parsetype_inplace(
                libab_ref_get(&to_resolve->children.data[index]), scope);
            index++;
        }
    }

    return result;
}

libab_result libab_resolve_parsetype_copy(libab_parsetype* to_resolve,
                                          libab_table* scope,
                                          libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_parsetype* parsetype;
    int is_placeholer = to_resolve->variant & LIBABACUS_TYPE_F_PLACE;
    if((parsetype = malloc(sizeof(*parsetype)))) {
        parsetype->variant = to_resolve->variant;
        if(!is_placeholer) {
            parsetype->variant |= LIBABACUS_TYPE_F_RESOLVED;
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        if(!is_placeholer) {
            libab_basetype* parent_basetype = libab_get_basetype(to_resolve, scope);
            if(parent_basetype) {
                parsetype->data_u.base = parent_basetype;
            } else {
                result = LIBAB_UNKNOWN_TYPE;
            }
        } else {
            result = libab_copy_string(&parsetype->data_u.name, to_resolve->data_u.name);
        }
    }


    if(result == LIBAB_SUCCESS && (to_resolve->variant & LIBABACUS_TYPE_F_PARENT)) {
        result = libab_ref_vec_init(&parsetype->children);
    }

    if(result == LIBAB_SUCCESS && (to_resolve->variant & LIBABACUS_TYPE_F_PARENT)) {
        size_t i;
        libab_ref temp;
        libab_ref new_type;
        for(i = 0; i < to_resolve->children.size && result == LIBAB_SUCCESS; i++) {
            libab_ref_vec_index(&to_resolve->children, i, &temp);
            result = libab_resolve_parsetype_copy(libab_ref_get(&temp), scope, &new_type);
            if(result == LIBAB_SUCCESS) {
                libab_ref_vec_insert(&parsetype->children, &new_type);
            }
            libab_ref_free(&new_type);
            libab_ref_free(&temp);
        }

        if(result != LIBAB_SUCCESS) {
            libab_ref_vec_free(&parsetype->children);
        }
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, parsetype, libab_free_parsetype);
        if(result != LIBAB_SUCCESS) libab_parsetype_free(parsetype);
    }

    if(result != LIBAB_SUCCESS) {
        free(parsetype);
        libab_ref_null(into);
    }

    return result;
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
        result = libab_ref_new(into, parsetype, libab_free_parsetype);
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

void _gc_visit_table_entry(libab_table_entry* entry, libab_visitor_function_ptr visitor, void* data) {
    if (entry->variant == ENTRY_VALUE) {
        libab_gc_visit(&entry->data_u.value, visitor, data);
    }
}

void _gc_visit_table_trie(libab_trie_node* parent, libab_visitor_function_ptr visitor, void* data) {
    ll_node* head;
    if(parent == NULL) return;
    head = parent->values.head;
    _gc_visit_table_trie(parent->child, visitor, data);
    _gc_visit_table_trie(parent->next, visitor, data);
    while(head != NULL) {
        _gc_visit_table_entry(head->data, visitor, data);
        head = head->next;
    }
}

void _gc_visit_table_children(void* parent, libab_visitor_function_ptr visitor, void* data) {
    libab_table* table = parent;
    libab_gc_visit(&table->parent, visitor, data);
    _gc_visit_table_trie(table->trie.head, visitor, data);
}

libab_result libab_create_table(libab* ab, libab_ref* into, libab_ref* parent) {
    libab_table* table;
    libab_result result = LIBAB_SUCCESS;
    if ((table = malloc(sizeof(*table)))) {
        libab_table_init(table);
        libab_table_set_parent(table, parent);
        result = libab_ref_new(into, table, libab_free_table);

        if (result != LIBAB_SUCCESS) {
            libab_free_table(table);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    } else {
        libab_gc_add(into, _gc_visit_table_children, &ab->containers);
    }
    return result;
}

void _gc_visit_value_children(void* val, libab_visitor_function_ptr visitor, void* data) {
    libab_value* value = val;
    libab_gc_visit(&value->data, visitor, data);
}

libab_result libab_create_value_ref(libab* ab, libab_ref* into, 
                                    libab_ref* data, libab_ref* type) {
    libab_value* value;
    libab_result result = LIBAB_SUCCESS;
    if ((value = malloc(sizeof(*value)))) {
        libab_value_init_ref(value, data, type);
        result = libab_ref_new(into, value, libab_free_value);

        if (result != LIBAB_SUCCESS) {
            libab_free_value(value);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    } else {
        libab_gc_add(into, _gc_visit_value_children, &ab->containers);
    }
    return result;
}

libab_result libab_create_value_raw(libab* ab, libab_ref* into,
                                    void* data, libab_ref* type) {
    libab_value* value;
    libab_result result = LIBAB_SUCCESS;

    if ((value = malloc(sizeof(*value)))) {
        result = libab_value_init_raw(value, data, type);
    } else {
        result = LIBAB_MALLOC;
    }

    if (result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, value, libab_free_value);
        if (result != LIBAB_SUCCESS) {
            libab_value_free(value);
        }
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(value);
    } else {
        libab_gc_add(into, _gc_visit_value_children, &ab->containers);
    }

    return result;
}

libab_result _create_value_function_list(libab* ab, libab_ref* into, libab_ref* type) {
    libab_ref list_ref;
    libab_result result = libab_create_function_list(ab, &list_ref, type);
    libab_ref_null(into);
    if (result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_create_value_ref(ab, into, &list_ref, type);
    }
    libab_ref_free(&list_ref);
    return result;
}


libab_result _register_function_existing(libab* ab,
                                         libab_table_entry* entry,
                                         libab_ref* function_val) {
    libab_value* old_value;
    libab_parsetype* old_type;
    libab_result result = LIBAB_SUCCESS;

    old_value = libab_ref_get(&entry->data_u.value);
    old_type = libab_ref_get(&old_value->type);

    if (old_type->data_u.base == libab_get_basetype_function_list(ab)) {
        libab_function_list* list = libab_ref_get(&old_value->data);
        result = libab_function_list_insert(list, function_val);
    } else if (old_type->data_u.base == libab_get_basetype_function(ab)) {
        libab_ref new_list;
        result =
            _create_value_function_list(ab, &new_list, &ab->type_function_list);
        if (result == LIBAB_SUCCESS) {
            libab_function_list* list =
                libab_ref_get(&((libab_value*)libab_ref_get(&new_list))->data);
            result = libab_function_list_insert(list, &entry->data_u.value);
            if (result == LIBAB_SUCCESS) {
                result = libab_function_list_insert(list, function_val);
            }
        }
        if (result == LIBAB_SUCCESS) {
            libab_ref_swap(&entry->data_u.value, &new_list);
        }
        libab_ref_free(&new_list);
    } else {
        libab_ref_swap(&entry->data_u.value, function_val);
    }

    return result;
}

libab_result _register_function_new(libab* ab, const char* name,
                                          libab_ref* function_val) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* entry;
    if ((entry = malloc(sizeof(*entry)))) {
        entry->variant = ENTRY_VALUE;
        libab_ref_copy(function_val, &entry->data_u.value);
        result = libab_table_put(libab_ref_get(&ab->table), name, entry);

        if (result != LIBAB_SUCCESS) {
            libab_table_entry_free(entry);
            free(entry);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    return result;
}


libab_result libab_overload_function(libab* ab,
                                     libab_table* table,
                                     const char* name,
                                     libab_ref* function) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* existing_entry = libab_table_search_filter(
            table, name, NULL, libab_table_compare_value);

    if (existing_entry) {
        result = _register_function_existing(ab, existing_entry,
                function);
    } else {
        result = _register_function_new(ab, name, function);
    }

    return result;
}

libab_result libab_set_variable(libab_table* table,
                                const char* name,
                                libab_ref* value) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* value_entry = libab_table_search_entry_value(table, name);
    if(value_entry) {
        libab_ref_free(&value_entry->data_u.value);
        libab_ref_copy(value, &value_entry->data_u.value);
    } else {
        result = libab_put_table_value(table, name, value);
    }
    return result;
}

void _gc_visit_function_children(void* function, libab_visitor_function_ptr visitor, void* data) {
    size_t index = 0;
    libab_function* func = function;
    libab_gc_visit(&func->scope, visitor, data);
    for(; index < func->params.size; index++) {
        libab_gc_visit(&func->params.data[index], visitor, data);
    }
}

libab_result libab_create_function_internal(libab* ab, libab_ref* into,
                                            void (*free_function)(void*),
                                            libab_function_ptr fun,
                                            libab_ref* scope) {
    libab_function* new_function;
    libab_result result = LIBAB_SUCCESS;

    if ((new_function = malloc(sizeof(*new_function)))) {
        result = libab_function_init_internal(new_function, fun, scope);
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
    } else {
        libab_gc_add(into, _gc_visit_function_children, &ab->containers);
    }

    return result;
}

libab_result libab_create_function_tree(libab* ab, libab_ref* into,
                                        void (*free_function)(void*),
                                        libab_tree* tree,
                                        libab_ref* scope) {
    libab_function* new_function;
    libab_result result = LIBAB_SUCCESS;

    if ((new_function = malloc(sizeof(*new_function)))) {
        result = libab_function_init_tree(new_function, tree, scope);
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
    } else {
        libab_gc_add(into, _gc_visit_function_children, &ab->containers);
    }

    return result;
}

libab_result libab_create_function_behavior(libab* ab, libab_ref* into,
                                            void (*free_function)(void*),
                                            libab_behavior* behavior,
                                            libab_ref* scope) {
    libab_function* new_function;
    libab_result result = LIBAB_SUCCESS;

    if((new_function = malloc(sizeof(*new_function)))) {
        result = libab_function_init_behavior(new_function, behavior, scope);
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, new_function, free_function);
        if(result != LIBAB_SUCCESS) {
            libab_function_free(new_function);
        }
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
        free(new_function);
    } else {
        libab_gc_add(into, _gc_visit_function_children, &ab->containers);
    }

    return result;
}


void _gc_visit_function_list_children(void* list, libab_visitor_function_ptr visitor, void* data) {
    size_t index = 0;
    libab_function_list* func_list = list;
    for(; index < func_list->functions.size; index++) {
        libab_gc_visit(&func_list->functions.data[index], visitor, data);
    }
}

libab_result libab_create_function_list(libab* ab, libab_ref* into, libab_ref* type) {
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
    } else {
        libab_gc_add(into, _gc_visit_function_list_children, &ab->containers);
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

libab_basetype* libab_get_basetype(libab_parsetype* type, libab_table* scope) {
    libab_ref type_param;
    libab_basetype* to_return;
    if(type->variant & LIBABACUS_TYPE_F_RESOLVED) {
        to_return = type->data_u.base;
    } else {
        to_return = libab_table_search_basetype(scope, type->data_u.name);
        if(to_return == NULL) {
            libab_table_search_type_param(scope, type->data_u.name, &type_param);
            to_return = ((libab_parsetype*)libab_ref_get(&type_param))->data_u.base;
            libab_ref_free(&type_param);
        }
    }
    return to_return;
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

void libab_sanitize(char* to, const char* from, size_t buffer_size) {
    size_t index = 0;
    while (*from && index < (buffer_size - 2)) {
        if (*from == '+' || *from == '*' || *from == '\\' ||
                *from == '|' || *from == '[' || *from == ']' || *from == '(' ||
                *from == ')' || *from == '.')
            to[index++] = '\\';
        to[index++] = *(from++);
    }
    to[index] = '\0';
}

libab_result libab_create_instance(libab** into, 
                                   void* (*parse_function)(const char*),
                                   void (*free_function)(void*)) {
    libab_result result = LIBAB_SUCCESS;
    if((*into = malloc(sizeof(**into)))) {
        result = libab_init(*into, parse_function, free_function);
        if(result != LIBAB_SUCCESS) {
            free(*into);
            *into = NULL;
        }
    } else {
        result = LIBAB_MALLOC;
    }
    return result;
}
void libab_destroy_instance(libab* into) {
    libab_free(into);
    free(into);
}
