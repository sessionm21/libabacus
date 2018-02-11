#include "table.h"
#include <stdlib.h>

void table_init(table* table) {
    ht_init(&table->table);
    table->parent = NULL;
} 

void table_free(table* table) {
    ht_free(&table->table);
}
