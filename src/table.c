#include "table.h"
#include "lexer.h"
#include "util.h"
#include <stdlib.h>

void libab_table_init(libab_table* table) {
    libab_trie_init(&table->trie);
    libab_ref_null(&table->parent);
}
libab_table_entry* libab_table_search_filter(libab_table* table,
                                             const char* string, void* data,
                                             compare_func compare) {
    void* to_return = NULL;
    do {
        const ll* matches = libab_trie_get(&table->trie, string);
        to_return = ll_find(matches, data, compare);
        table = libab_ref_get(&table->parent);
    } while (table && to_return == NULL);
    return to_return;
}
libab_table_entry* libab_table_search(libab_table* table, const char* string) {
    void* to_return = NULL;
    do {
        to_return = ll_head(libab_trie_get(&table->trie, string));
        table = libab_ref_get(&table->parent);
    } while (table && to_return == NULL);
    return to_return;
}

#define OP_TYPE_COMPARATOR(NAME, TYPE)                                         \
    int NAME(const void* left, const void* right) {                            \
        const libab_table_entry* entry = right;                                \
        return entry->variant == ENTRY_OP && entry->data_u.op.variant == TYPE; \
    }

OP_TYPE_COMPARATOR(libab_table_compare_op_prefix, OPERATOR_PREFIX)
OP_TYPE_COMPARATOR(libab_table_compare_op_infix, OPERATOR_INFIX)
OP_TYPE_COMPARATOR(libab_table_compare_op_postfix, OPERATOR_POSTFIX)

int libab_table_compare_value(const void* left, const void* right) {
    const libab_table_entry* entry = right;
    return entry->variant == ENTRY_VALUE;
}

int libab_table_compare_basetype(const void* left, const void* right) {
    const libab_table_entry* entry = right;
    return entry->variant == ENTRY_BASETYPE;
}

int libab_table_compare_type_param(const void* left, const void* right) {
    const libab_table_entry* entry = right;
    return entry->variant == ENTRY_TYPE_PARAM;
}

libab_operator* libab_table_search_operator(libab_table* table,
                                            const char* string, int type) {
    libab_table_entry* entry = NULL;
    if (type == OPERATOR_PREFIX) {
        entry = libab_table_search_filter(table, string, NULL,
                                          libab_table_compare_op_prefix);
    } else if (type == OPERATOR_INFIX) {
        entry = libab_table_search_filter(table, string, NULL,
                                          libab_table_compare_op_infix);
    } else if (type == OPERATOR_PREFIX) {
        entry = libab_table_search_filter(table, string, NULL,
                                          libab_table_compare_op_postfix);
    }
    return entry ? &entry->data_u.op : NULL;
}

libab_basetype* libab_table_search_basetype(libab_table* table,
                                            const char* string) {
    libab_table_entry* entry = libab_table_search_filter(
        table, string, NULL, libab_table_compare_basetype);
    return entry ? entry->data_u.basetype : NULL;
}

void libab_table_search_value(libab_table* table, const char* string,
                              libab_ref* ref) {
    libab_table_entry* entry = libab_table_search_filter(
        table, string, NULL, libab_table_compare_value);
    if (entry) {
        libab_ref_copy(&entry->data_u.value, ref);
    } else {
        libab_ref_null(ref);
    }
}

void libab_table_search_type_param(libab_table* table, const char* string,
                                   libab_ref* ref) {
    libab_table_entry* entry = libab_table_search_filter(
        table, string, NULL, libab_table_compare_type_param);
    if (entry) {
        libab_ref_copy(&entry->data_u.type_param, ref);
    } else {
        libab_ref_null(ref);
    }
}

libab_result libab_table_put(libab_table* table, const char* string,
                             libab_table_entry* entry) {
    return libab_trie_put(&table->trie, string, entry);
}
int _table_foreach_entry_free(void* data, va_list args) {
    libab_table_entry_free(data);
    free(data);
    return 0;
}
void libab_table_set_parent(libab_table* table, libab_ref* parent) {
    libab_ref_free(&table->parent);
    libab_ref_copy(parent, &table->parent);
}
void libab_table_free(libab_table* table) {
    libab_trie_foreach(&table->trie, NULL, compare_always,
                       _table_foreach_entry_free);
    libab_trie_free(&table->trie);
    libab_ref_free(&table->parent);
}
void libab_table_entry_free(libab_table_entry* entry) {
    if (entry->variant == ENTRY_OP) {
        libab_operator_free(&entry->data_u.op);
    } else if (entry->variant == ENTRY_BASETYPE) {
        libab_basetype_free(entry->data_u.basetype);
    } else if (entry->variant == ENTRY_VALUE) {
        libab_ref_free(&entry->data_u.value);
    } else if (entry->variant == ENTRY_TYPE_PARAM) {
        libab_ref_free(&entry->data_u.type_param);
    }
}
