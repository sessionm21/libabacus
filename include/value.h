#ifndef LIBABACUS_VALUE_H
#define LIBABACUS_VALUE_H

#include "type.h"
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
    void* data;
};

typedef struct libab_value_s libab_value;

/**
 * Initializes a new value with the given allocated memory for the data,
 * and the given type.
 * @param data the data for this value. It is freed when the value is released
 * according to the free function of the base type.
 * @param type the type of this value.
 */
void libab_value_init(libab_value* value, void* data, libab_ref* type);
/**
 * Frees the given value.
 * @param value the value to free.
 */
void libab_value_free(libab_value* value);

#endif
