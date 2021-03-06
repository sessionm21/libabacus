#ifndef LIBABACUS_UTIL_H
#define LIBABACUS_UTIL_H

#include "function_list.h"
#include "libds.h"
#include "liblex.h"
#include "parsetype.h"
#include "result.h"
#include "table.h"
#include "libabacus.h"
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
libab_result libab_resolve_parsetype_inplace(libab_parsetype* to_resolve,
                                     libab_table* scope);
/**
 * Resolves the given parsetype, looking through the scope to find
 * all referenced base types, and creates a copy with these basetypes.
 * @param to_resolve the parsetype to resolve.
 * @param scope the scope to use for resolving the type info.
 * @return the result of the operation.
 */
libab_result libab_resolve_parsetype_copy(libab_parsetype* to_resolve,
                                          libab_table* scope,
                                          libab_ref* into);
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
 * @param ab the libabacus instance in which the table is being created.
 * @param into the reference to store the table into.
 * @param parent the parent reference to store.
 * @return the result of the instantiation.
 */
libab_result libab_create_table(libab* ab, libab_ref* into, libab_ref* parent);
/**
 * Allocates a new reference counted value with the given type and data.
 * @param into the reference to store the allocated data into.
 * @param data the type-specific data this value holds.
 * @param type the type to give the value.
 * @return the result of necessary allocations.
 */
libab_result libab_create_value_ref(libab* ab, libab_ref* into, 
                                    libab_ref* data, libab_ref* type);
/**
 * Allocates a new reference counted value with the given type and data.
 * @param into the reference to store the allocated data into.
 * @param data the type-specific data this value holds.
 * @param type the type to give the value.
 * @return the result of necessary allocations.
 */
libab_result libab_create_value_raw(libab* ab, libab_ref* into,
                                    void* data, libab_ref* type);
/**
 * Overloads a function of the given name.
 * @param table the table to insert the function into.
 * @param name the name of the function.
 * @param function the function to register.
 * @return the result of the overload.
 */
libab_result libab_overload_function(libab* ab,
                                     libab_table* table,
                                     const char* name,
                                     libab_ref* function);
/**
 * Sets a value under the given name, overriding
 * an existing value if it exists.
 * @param table the table to store the value into.
 * @param name the name of the variable.
 * @param the value of the new variable.
 * @return the result of setting the variable.
 */
libab_result libab_set_variable(libab_table* table,
                                const char* name,
                                libab_ref* value);
/**
 * Allocates a function that uses internal code to run.
 * @param into the reference into which to store the new function.
 * @param free_function the free function used to free function instances.
 * @param fun the function implementation.
 * @param scope the scope in which this function was declared.
 * @return libab_result the result of any necessary allocations.
 */
libab_result libab_create_function_internal(libab* ab, libab_ref* into,
                                            void (*free_function)(void*),
                                            libab_function_ptr fun,
                                            libab_ref* scope);
/**
 * Allocates a function that uses a tree to run.
 * @param into the reference into which to store the new function.
 * @param free_function the free function used to free function instances.
 * @param tree the function implementation.
 * @param scope the scope in which this function was declared.
 * @return libab_result the result of any necessary allocations.
 */
libab_result libab_create_function_tree(libab* ab, libab_ref* into,
                                        void (*free_function)(void*),
                                        libab_tree* tree,
                                        libab_ref* scope);
/**
 * Allocates a function that uses a given behavior to run.
 * @param into the reference into which to store the new function.
 * @param free_function the free function used to free function instances.
 * @param behavior the behavior that dictates what the function does.
 * @param scope the scope in which this function was declared.
 * @return libab_result the result of any necessary allocations.
 */
libab_result libab_create_function_behavior(libab* ab, libab_ref* into,
                                            void (*free_function)(void*),
                                            libab_behavior* behavior,
                                            libab_ref* scope);
/**
 * Creates a function list object, storing it in to the given reference.
 * @param into the reference to store into.
 * @param the function_list type.
 * @return the result of the allocations.
 */
libab_result libab_create_function_list(libab* ab, libab_ref* into, libab_ref* type);
/**
 * Creates a new table entry that holds the given value.
 * @param table the table to store the entry into.
 * @param key the key under which to store the value.
 * @param value the value to store into the table.
 * @param result the result of the operation.
 */
libab_result libab_put_table_value(libab_table* table, const char* key,
                                   libab_ref* value);
/**
 * Gets the basetype of a parsetype.
 * @param type the parsetype to get the basetype of.
 * @param scope the scope to use for searches.
 * @return the basetype.
 */
libab_basetype* libab_get_basetype(libab_parsetype* type, libab_table* scope);
/**
 * Returns the data stored in the given reference to a libab_value.
 * This is not the same as libab_ref_get: libab_ref_get will return directly
 * the data pointed to by the ference. libab_unwrap_value assumes the reference
 * it's given is that to a libab_value, extracts it, then extracts its data
 * field.
 * @param ref the reference to unwrap.
 * @return the resulting data.
 */
void* libab_unwrap_value(libab_ref* ref);
/**
 * Similar to unwrap_value; Assumes the vector is a vector of
 * values, and unwraps the value at the given index.
 * @param vec the vector to unwrap a value from.
 * @param index the index to look up.
 * @return the value at the given index.
 */
void* libab_unwrap_param(libab_ref_vec* vec, size_t index);
/**
 * Sanitizes a string to avoid liblex compilation errors.
 * @param to the buffer to store the sanitized string to.
 * @param from the string to sanitize.
 * @param buffer_size the size of the to buffer.
 */
void libab_sanitize(char* to, const char* from, size_t buffer_size);
/**
 * Creates a new instance of libabacus allocated on the heap.
 * @param into the pointer into which to store the newly allocated libab instance.
 * @param parse_function the function used to parse numbers.
 * @param free_function the function used to free parsed numbers.
 */
libab_result libab_create_instance(libab** into, 
                                   void* (*parse_function)(const char*),
                                   void (*free_function)(void*));
/**
 * Destroys the given libabacus instance allocated on the stack.
 * @param into the reference to destroy.
 */
void libab_destroy_instance(libab* into);

#endif
