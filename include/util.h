#ifndef LIBABACUS_UTIL_H
#define LIBABACUS_UTIL_H

#include "libds.h"
#include "liblex.h"
#include "result.h"
#include "parsetype.h"
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
 * Copies a range of the given string into a new, null-terminated string allocated
 * on the heap.
 * @param destination the pointer to populate with the newly created string.
 * @param source the source from which to pull character information from.
 * @param from the index (inclusive) at which to begin copying.
 * @param to the index (exclusive) at which to end copying.
 * @return the result of the operation.
 */
libab_result libab_copy_string_range(char** destination, const char* source, size_t from, size_t to);
/**
 * Copies the given string, starting at 0 and copying length bytes.
 * @param destination the pointer to populate with the newly created string.
 * @param source to source string to copy.
 * @param length the number of bytes to copy.
 * @return the result of the operation.
 */
libab_result libab_copy_string_size(char** destination, const char* source, size_t length);
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
libab_result libab_resolve_parsetype(libab_parsetype* to_resolve, libab_table* scope);

#endif
