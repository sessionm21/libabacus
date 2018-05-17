#ifndef LIBABACUS_UTIL_H
#define LIBABACUS_UTIL_H

#include "function_list.h"
#include "libds.h"
#include "liblex.h"
#include "parsetype.h"
#include "result.h"
#include "table.h"
#include <string.h>

/**
 * Converts a result code from liblex to libabacus.
 * @param to_convert the code to convert.
 * @return the libabacus equivalent of the error code.
 */
libab_result libab_convert_lex_result(liblex_result to_convert);
/**
 * Converts a result code from libds to libabacus.
 * @param to_convert the code to convert.
 * @return the libabacus equivalent of the error code.
 */
libab_result libab_convert_ds_result(libds_result to_convert);
/**
 * Copies a range of the given string into a new, null-terminated string
 * allocated on the heap.
 * @param destination the pointer to populate with the newly created string.
 * @param source the source from which to pull character information from.
 * @param from the index (inclusive) at which to begin copying.
 * @param to the index (exclusive) at which to end copying.
 * @return the result of the operation.
 */
libab_result libab_copy_string_range(char** destination, const char* source,
                                     size_t from, size_t to);
/**
 * Copies the given string, starting at 0 and copying length bytes.
 * @param destination the pointer to populate with the newly created string.
 * @param source to source string to copy.
 * @param length the number of bytes to copy.
 * @return the result of the operation.
 */
libab_result libab_copy_string_size(char** destination, const char* source,
                                    size_t length);
/**
 * Copies the entire string into a null-terminated string allocated
 * on the heap.
 * @param destination the pointer to populate with the newly allocated string.
 * @param source the source string to copy.
 * @return the result of the operation.
 */
libab_result libab_copy_string(char** destination, const char* source);
/**
 * Resolves the given parsetype, looking through the scope to find all the
 * referenced base types, if applicable.
 * @param to_resolve the parsetype to resolve.
 * @param scope the scope to use for resolving the type info.
 */
libab_result libab_resolve_parsetype(libab_parsetype* to_resolve,
                                     libab_table* scope);
/**
 * Creates a new type instance, and stores it into the given reference.
 * @param to_instantiate the basetype to instantiate.
 * @param into the reference to store the new type into.
 * @param n the number of type parameters.
 * @return the result of the instantiation.
 */
libab_result libab_instantiate_basetype(libab_basetype* to_instantiate,
                                        libab_ref* into, size_t n, ...);
/**
 * Creates a new libab_table, and stores it into the given reference.
 * @param into the reference to store the table into.
 * @param parent the parent reference to store.
 * @return the result of the instantiation.
 */
libab_result libab_create_table(libab_ref* into, libab_ref* parent);
/**
 * Allocates a new reference counted value with the given type and data.
 * @param into the reference to store the allocated data into.
 * @param data the type-specific data this value holds.
 * @param type the type to give the value.
 * @return the result of necessary allocations.
 */
libab_result libab_create_value_ref(libab_ref* into, libab_ref* data,
                                    libab_ref* type);
/**
 * Allocates a new reference counted value with the given type and data.
 * @param into the reference to store the allocated data into.
 * @param data the type-specific data this value holds.
 * @param type the type to give the value.
 * @return the result of necessary allocations.
 */
libab_result libab_create_value_raw(libab_ref* into, void* data,
                                    libab_ref* type);
/**
 * Allocates a function that uses internal code to run.
 * @param into the reference into which to store the new function.
 * @param free_function the free function used to free function instances.
 * @param fun the function implementation.
 * @return libab_result the result of any necessary allocations.
 */
libab_result libab_create_function_internal(libab_ref* into,
                                            void (*free_function)(void*),
                                            libab_function_ptr fun);
/**
 * Allocates a function that uses a tree to run.
 * @param into the reference into which to store the new function.
 * @param free_function the free function used to free function instances.
 * @param tree the function implementation.
 * @return libab_result the result of any necessary allocations.
 */
libab_result libab_create_function_tree(libab_ref* into,
                                        void (*free_function)(void*),
                                        libab_tree* tree);
/**
 * Creates a function list object, storing it in to the given reference.
 * @param into the reference to store into.
 * @param the function_list type.
 * @return the result of the allocations.
 */
libab_result libab_create_function_list(libab_ref* into, libab_ref* type);
/**
 * Creates a new table entry that holds the given value.
 * @param table the table to store the entry into.
 * @param key the key under which to store the value.
 * @param value the value to store into the table.
 * @param result the result of the operation.
 */
libab_result libab_put_table_value(libab_table* table, const char* key,
                                   libab_ref* value);

#endif
