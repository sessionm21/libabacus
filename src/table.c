#include "table.h"
#include <stdlib.h>
#include "util.h"
#include "lexer.h"

void libab_table_init(libab_table* table) {
    libab_trie_init(&table->trie);
    table->parent = NULL;
} 
libab_table_entry* libab_table_search_filter(libab_table* table, const char* string, void* data, compare_func compare) {
    void* to_return = NULL;
    do {
        const ll* matches = libab_trie_get(&table->trie, string);
        to_return = ll_find(matches, data, compare);
        table = table->parent;
    } while(table && to_return == NULL);
    return to_return;
}
libab_table_entry* libab_table_search(libab_table* table, const char* string) {
    void* to_return = NULL;
    do {
        to_return = ll_head(libab_trie_get(&table->trie, string));
        table = table->parent;
    } while(table && to_return == NULL);
    return to_return;
}

#define OP_TYPE_COMPARATOR(NAME, TYPE) int NAME(const void* left, const void* right) {\
    const libab_table_entry* entry = right;\
    return entry->variant == ENTRY_OP && entry->data_u.op.type == TYPE;\
}

OP_TYPE_COMPARATOR(_table_compare_prefix, OPERATOR_PREFIX)
OP_TYPE_COMPARATOR(_table_compare_infix, OPERATOR_INFIX)
OP_TYPE_COMPARATOR(_table_compare_postfix, OPERATOR_POSTFIX)

libab_operator* libab_table_search_operator(libab_table* table, const char* string, int type) {
    libab_table_entry* entry = NULL;
    if(type == OPERATOR_PREFIX) {
        entry = libab_table_search_filter(table, string, NULL, _table_compare_prefix);
    } else if(type == OPERATOR_INFIX) {
        entry = libab_table_search_filter(table, string, NULL, _table_compare_infix);
    } else if(type == OPERATOR_PREFIX) {
        entry = libab_table_search_filter(table, string, NULL, _table_compare_postfix);
    }
    return entry ? &entry->data_u.op : NULL;
}

int _table_compare_function(const void* left, const void* right) {
    const libab_table_entry* entry = right;
    return entry->variant == ENTRY_FUN;
}

libab_function* libab_table_search_function(libab_table* table, const char* string) {
    libab_table_entry* entry = libab_table_search_filter(table, string, NULL, _table_compare_function);
    return entry ? &entry->data_u.function : NULL;
}
libab_result libab_table_put(libab_table* table, const char* string, libab_table_entry* entry) {
    return libab_trie_put(&table->trie, string, entry);
}
int _table_foreach_entry_free(void* data, va_list args) {
    libab_table_entry_free(data);
    free(data);
    return 0;
}
void libab_table_free(libab_table* table) {
    libab_trie_foreach(&table->trie, NULL, compare_always, _table_foreach_entry_free);
    libab_trie_free(&table->trie);
}
void libab_table_entry_free(libab_table_entry* entry) {
    if(entry->variant == ENTRY_OP) {
        libab_parsetype_free_recursive(entry->data_u.op.behavior.type);
    } else if(entry->variant == ENTRY_FUN) {
        libab_parsetype_free_recursive(entry->data_u.function.behavior.type);
    }
}
