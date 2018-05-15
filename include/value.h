#ifndef LIBABACUS_VALUE_H
#define LIBABACUS_VALUE_H

#include "refcount.h"
#include "result.h"

/**
 * A struct that represents a value.
 */
struct libab_value_s {
    /**
     * The type of the value.
     */
    libab_ref type;
    /**
     * The data that is specific to this value.
     */
    libab_ref data;
};

typedef struct libab_value_s libab_value;

/**
 * Initializes a new value with the given reference counted data
 * and the given type.
 * @param value the value to initialize.
 * @param data the data for this value. Its refcount is decreased when the value is freed.
 * @param type the type of this value.
 */
void libab_value_init_ref(libab_value* value, libab_ref* data, libab_ref* type);
/**
 * Initializes a new value with the given raw allocated data, and a type,
 * whose basetype's free function is used to release the data when the value is freed.
 * @param value the value to initialize.
 * @param data the data this value holds.
 * @param type the type of this value.
 * @return the result of any necessary allocations.
 */
libab_result libab_value_init_raw(libab_value* value, void* data, libab_ref* type);
/**
 * Frees the given value.
 * @param value the value to free.
 */
void libab_value_free(libab_value* value);

#endif
