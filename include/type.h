#ifndef LIBABACUS_TYPE_H
#define LIBABACUS_TYPE_H

#include "ref_vec.h"
#include "basetype.h"

/**
 * A type instance. This is created at runtime
 * for every value that has a type, and represents an instance
 * of a basetype with concrete (though possibly templated) parameters.
 */
struct libab_type_s {
    /**
     * The base type that this type is an instance of.
     */
    libab_basetype* base;
    /**
     * The list of parameters this type holds.
     */
    libab_ref_vec params;
};

typedef struct libab_type_s libab_type;

/**
 * Initializes a given type with the given basetype.
 * @param type the type to initialize.
 * @param base the base type to use.
 * @return the result of the initialization.
 */
libab_result libab_type_init(libab_type* type, libab_basetype* base);
/**
 * Frees the memory allocated by the given type.
 * @param type the type to free.
 */
void libab_type_free(libab_type* type);

#endif
