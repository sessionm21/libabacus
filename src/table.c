#include "table.h"
#include <stdlib.h>

void table_init(table* table) {
    ht_init(&table->table);
    table->parent = NULL;
} 
table_entry* table_search(table* table, const char* string) {
    void* to_return = NULL;
    do {
        to_return = ht_get(&table->table, string);
        table = table->parent;
    } while(table && to_return == NULL);
    return to_return;
}
void table_free(table* table) {
    ht_free(&table->table);
}
