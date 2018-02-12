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
libab_result libab_table_put(libab_table* table, const char* string, libab_table_entry* entry) {
    return libab_convert_ds_result(ht_put(&table->table, string, entry));
}
void table_free(libab_table* table) {
    ht_free(&table->table);
}
