#ifndef LIBABACUS_TABLE_H
#define LIBABACUS_TABLE_H

#include "ht.h"
#include "libabacus.h"

/**
 * A struct that represents a structure
 * similar to a symbol table. This structure
 * is used to keep track of definitions such
 * as types, functions, and variables in an
 * environment with scopes.
 */
struct table {
    /**
     * The "parent" scope of this table.
     */
    struct table* parent;
    /**
     * The hash table used to store the data.
     */
    ht table;
};

/**
 * Enum that represents the type of a table
 * entry.
 */
enum table_entry_variant {
    ENTRY_VALUE,
    ENTRY_TYPE,
    ENTRY_FUNCTION
};

/**
 * An entry in the table.
 */
struct table_entry {
    /**
     * The type of this entry.
     */
    enum table_entry_variant variant;
};

typedef struct table table;
typedef enum table_entry_variant table_entry_variant;
typedef struct table_entry table_entry;

/**
 * Initializes the given table.
 * @param table the table to initialize.
 */
void table_init(table* table);
/**
 * Frees the resources allocated by the
 * given table.
 * @param table the table to free.
 */
void table_free(table* table);

#endif
