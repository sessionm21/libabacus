#ifndef LIBABACUS_TABLE_H
#define LIBABACUS_TABLE_H

#include "ht.h"
#include "result.h"
#include "custom.h"

/**
 * A struct that represents a structure
 * similar to a symbol table. This structure
 * is used to keep track of definitions such
 * as types, functions, and variables in an
 * environment with scopes.
 */
struct libab_table_s {
    /**
     * The "parent" scope of this table.
     */
    struct libab_table_s* parent;
    /**
     * The hash table used to store the data.
     */
    ht table;
};

/**
 * Enum that represents the type of a table
 * entry.
 */
enum libab_table_entry_variant_e {
    ENTRY_VALUE,
    ENTRY_TYPE,
    ENTRY_OP,
    ENTRY_FUN
};

/**
 * An entry in the table.
 */
struct libab_table_entry_s {
    /**
     * The type of this entry.
     */
    enum libab_table_entry_variant_e variant;
    /**
     * Union that holds the various types of
     * data that this entry could hold.
     */
    union {
        libab_operator op;
        libab_function function;
    } data_u;
};

typedef struct libab_table_s libab_table;
typedef enum libab_table_entry_variant_e libab_table_entry_variant;
typedef struct libab_table_entry_s libab_table_entry;

/**
 * Initializes the given table.
 * @param table the table to initialize.
 */
void libab_table_init(libab_table* table);
/**
 * Searches for the given string in the table.
 * @param table the table to search.
 * @param string the string to search for.
 * @return the table entry, or NULL if an entry was not found.
 */
libab_table_entry* libab_table_search(libab_table* table, const char* string);
/**
 * Searches for the given string in the table, returning a value only
 * if it is an operator.
 * @param table the table to search.
 * @param string the string to search for.
 * @return the found operator, or NULL if it was not found.
 */
libab_operator* libab_table_search_operator(libab_table* table, const char* string);
/**
 * Searches for the given string in the table, returning a value only
 * if it is a function.
 * @param table the table to search.
 * @param string the string to search for.
 * @return the found function, or NULL if it was not found.
 */
libab_function* libab_table_search_function(libab_table* table, const char* string);
/**
 * Stores the given entry in the table under the given key.
 * @param table the table to store the entry into.
 * @param string the string to use as the key.
 * @param entry the new entry to put into the table.
 * @return the result of the insertion, which could be LIBAB_MALLOC.
 */
libab_result libab_table_put(libab_table* table, const char* string, libab_table_entry* entry);
/**
 * Frees the resources allocated by the
 * given table.
 * @param table the table to free.
 */
void libab_table_free(libab_table* table);

#endif
