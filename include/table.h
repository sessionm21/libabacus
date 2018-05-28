#ifndef LIBABACUS_TABLE_H
#define LIBABACUS_TABLE_H

#include "basetype.h"
#include "custom.h"
#include "refcount.h"
#include "result.h"
#include "trie.h"

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
    libab_ref parent;
    /**
     * The hash table used to store the data.
     */
    libab_trie trie;
};

/**
 * Enum that represents the type of a table
 * entry.
 */
enum libab_table_entry_variant_e { 
    ENTRY_VALUE, 
    ENTRY_BASETYPE,
    ENTRY_OP,
    ENTRY_TYPE_PARAM
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
        libab_basetype* basetype;
        libab_ref value;
        libab_ref type_param;
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
 * Searches for the given string in the table, comparing
 * values to a given reference as an extra filtering step.
 * @param table the table to search.
 * @param string the string to search for.
 * @param data the data to compare against potential values.
 * @param compare the comparison function to use.
 * @return the table entry, or NULL if an entry was not found.
 */
libab_table_entry* libab_table_search_filter(libab_table* table,
                                             const char* string, void* data,
                                             compare_func compare);
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
 * @param type the type of operator to search for (infix, prefix, postfix)
 * @return the found operator, or NULL if it was not found.
 */
libab_operator* libab_table_search_operator(libab_table* table,
                                            const char* string, int type);
/**
 * Searches for the given basetype in the table, returning a value
 * only if it's a basetype.
 * @param table the table to search.
 * @param string the string to search for.
 * @return the found basetype, or NULL if it was not found.
 */
libab_basetype* libab_table_search_basetype(libab_table* table,
                                            const char* string);
/**
 * Searches for the given value in the table.
 * @param table the table to search.
 * @param string the table entry key.
 * @param ref the reference to store the result into.
 */
void libab_table_search_value(libab_table* table, const char* string,
                              libab_ref* ref);
/**
 * Searches for the given type parameter in the talb.e
 * @param table the table to search in.
 * @param string the key ot search for.
 * @param ref the reference to store the type into.
 */
void libab_table_search_type_param(libab_table* table, const char* string,
                                   libab_ref* ref);
/**
 * Stores the given entry in the table under the given key.
 * @param table the table to store the entry into.
 * @param string the string to use as the key.
 * @param entry the new entry to put into the table.
 * @return the result of the insertion, which could be LIBAB_MALLOC.
 */
libab_result libab_table_put(libab_table* table, const char* string,
                             libab_table_entry* entry);
/**
 * Sets the parent of the given table.
 * @param table the table whose parent to set.
 * @param parent a valid reference to a parent table.
 */
void libab_table_set_parent(libab_table* table, libab_ref* parent);
/**
 * Frees the resources allocated by the
 * given table.
 * @param table the table to free.
 */
void libab_table_free(libab_table* table);
/**
 * Comparison function used to search the table for a prefix operator.
 */
int libab_table_compare_op_prefix(const void* left, const void* right);
/**
 * Comparison function used to search the table for a infix operator.
 */
int libab_table_compare_op_infix(const void* left, const void* right);
/**
 * Comparison function used to search the table for a postfix operator.
 */
int libab_table_compare_op_postfix(const void* left, const void* right);
/**
 * Comparison function used to search the table for a value.
 */
int libab_table_compare_value(const void* left, const void* right);
/**
 * Comparison function used to search the table for a basetype.
 */
int libab_table_compare_basetype(const void* left, const void* right);
/**
 * Frees the given table entry.
 * @param entry the entry to free.
 */
void libab_table_entry_free(libab_table_entry* entry);

#endif
