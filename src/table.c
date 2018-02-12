#include "table.h"
#include <stdlib.h>

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
void table_free(libab_table* table) {
    ht_free(&table->table);
}
