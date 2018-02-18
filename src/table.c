#include "table.h"
#include <stdlib.h>
#include "util.h"

void libab_table_init(libab_table* table) {
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
    if(entry && entry->variant == ENTRY_OP) {
        to_return = &entry->data_u.op;
    }
    return to_return;
}
libab_function* libab_table_search_function(libab_table* table, const char* string) {
    libab_table_entry* entry = table_search(table, string);
    libab_function* to_return = NULL;
    if(entry && entry->variant == ENTRY_FUN) {
        to_return = &entry->data_u.function;
    }
    return to_return;
}
libab_result libab_table_put(libab_table* table, const char* string, libab_table_entry* entry) {
    return libab_convert_ds_result(ht_put(&table->table, string, entry));
}
int _table_foreach_entry_free(void* data, va_list args) {
    libab_table_entry_free(data);
    free(data);
    return 0;
}
void libab_table_free(libab_table* table) {
    ht_foreach(&table->table, NULL, compare_always, _table_foreach_entry_free);
    ht_free(&table->table);
}
void libab_table_entry_free(libab_table_entry* entry) {
    
}
