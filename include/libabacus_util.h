#ifndef LIBABACUS_UTIL_H
#define LIBABACUS_UTIL_H

#include "libds.h"
#include "liblex.h"
#include "libabacus.h"

/**
 * Converts a result code from liblex to libabacus.
 * @param to_convert the code to convert.
 * @return the libabacus equivalent of the error code.
 */
libab_result convert_lex_result(liblex_result to_convert);
/**
 * Converts a result code from libds to libabacus.
 * @param to_convert the code to convert.
 * @return the libabacus equivalent of the error code.
 */
libab_result convert_ds_result(libds_result to_convert);

#endif
