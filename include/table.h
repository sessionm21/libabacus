#ifndef LIBABACUS_TABLE_H
#define LIBABACUS_TABLE_H

#include "ht.h"
#include "libabacus_result.h"

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
    ENTRY_FUNCTION
};

/**
 * An entry in the table.
 */
struct libab_table_entry_s {
    /**
     * The type of this entry.
     */
    enum libab_table_entry_variant_e variant;
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
 * Frees the resources allocated by the
 * given table.
 * @param table the table to free.
 */
void libab_table_free(libab_table* table);

#endif
