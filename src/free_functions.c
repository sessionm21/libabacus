#include "free_functions.h"
#include "custom.h"
#include "function_list.h"
#include "parsetype.h"
#include "table.h"
#include "value.h"
#include <stdlib.h>

void free_function(void* func) {
    libab_function_free(func);
    free(func);
}
void free_function_list(void* function_list) {
    libab_function_list_free(function_list);
    free(function_list);
}
void free_unit(void* unit) {

}
void free_parsetype(void* parsetype) {
    libab_parsetype_free(parsetype);
    free(parsetype);
}
void free_table(void* table) {
    libab_table_free(table);
    free(table);
}
void free_value(void* value) {
    libab_value_free(value);
    free(value);
}
