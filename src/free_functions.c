#include "free_functions.h"
#include "custom.h"
#include "function_list.h"
#include "parsetype.h"
#include "table.h"
#include "value.h"
#include <stdlib.h>

void libab_free_function(void* func) {
    libab_function_free(func);
    free(func);
}
void libab_free_function_list(void* function_list) {
    libab_function_list_free(function_list);
    free(function_list);
}
void libab_free_unit(void* unit) {

}
void libab_free_bool(void* b) {
    free(b);
}
void libab_free_parsetype(void* parsetype) {
    libab_parsetype_free(parsetype);
    free(parsetype);
}
void libab_free_table(void* table) {
    libab_table_free(table);
    free(table);
}
void libab_free_value(void* value) {
    libab_value_free(value);
    free(value);
}
