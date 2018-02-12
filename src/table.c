#include "table.h"
#include <stdlib.h>
#include "util.h"

void table_init(libab_table* table) {
    ht_init(&table->table);
    table->parent = NULL;
} 
libab_table_entry* table_search(libab_table* table, const char* string) {
    void* to_return = NULL;
    do {
        to_return = ht_get(&table->table, string);
        table = table->parent;
    } while(table && to_return == NULL);
    return to_return;
}
libab_operator* libab_table_search_operator(libab_table* table, const char* string) {
    libab_table_entry* entry = table_search(table, string);
    libab_operator* to_return = NULL;
    if(entry && entry->variant == ENTRY_OPERATOR) {
        to_return = &entry->data_u.op;
    }
    return to_return;
}
libab_function* libab_table_search_function(libab_table* table, const char* string) {
    libab_table_entry* entry = table_search(table, string);
    libab_function* to_return = NULL;
    if(entry && entry->variant == ENTRY_FUNCTION) {
        to_return = &entry->data_u.function;
    }
    return to_return;
}
libab_result libab_table_put(libab_table* table, const char* string, libab_table_entry* entry) {
    return libab_convert_ds_result(ht_put(&table->table, string, entry));
}
void table_free(libab_table* table) {
    ht_free(&table->table);
}
