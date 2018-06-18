#ifndef LIBABACUS_FREE_FUNCTIONS_H
#define LIBABACUS_FREE_FUNCTIONS_H

/**
 * Free functions. Because a lot of the reference
 * counting operations require free functions,
 * and redeclaring them in mutliple files makes no
 * sense (also, it doesn't link :^) ), we
 * put them all here.
 */

/**
 * Frees a libab_function.
 * @param func the function to free.
 */
void libab_free_function(void* func);
/**
 * Frees a libab_function_list.
 * @param func_list the function list to free.
 */
void libab_free_function_list(void* func_list);
/**
 * Frees a unit. This is a no-op.
 * @param unit the unit to free.
 */
void libab_free_unit(void* unit);
/**
 * Frees a parsetype.
 * @param parsetype the parsetype to free.
 */
void libab_free_parsetype(void* parsetype);
/**
 * Frees a table.
 * @param table the table to free.
 */
void libab_free_table(void* table);
/**
 * Frees a value.
 * @param value the value to free.
 */
void libab_free_value(void* value);

#endif
