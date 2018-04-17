#ifndef LIBABACUS_BASETYPE_H
#define LIBABACUS_BASETYPE_H

#include "vec.h"
#include "result.h"

/**
 * An enum that represents the various
 * types of the basetype parameters.
 */
enum libab_basetype_variant_e {
    BT_NAME,
    BT_LIST
};

/**
 * A struct that holds information about the basetype
 * parameter, such as a type name or a "..." (called a list).
 */
struct libab_basetype_param_s {
    /**
     * The variant of this type parameter.
     */
    enum libab_basetype_variant_e variant;
    /**
     * The name of the parameter, if one is appropriate.
     */
    const char* name;
};

/**
 * A struct that holds all the information about a base type.
 */
struct libab_basetype_s {
    /**
     * The function used to free the value.
     */
    void (*free_function)(void*);
    /**
     * The parameters of the type. This is a constant array, and
     * doesn't need memory allocation.
     */
    const struct libab_basetype_param_s* params;
    /**
     * The size of the param aray.
     */
    int count;
};

typedef enum libab_basetype_variant_e libab_basetype_variant;
typedef struct libab_basetype_param_s libab_basetype_param;
typedef struct libab_basetype_s libab_basetype;

/**
 * Initializes a base type, including all the parameters that it needs
 * to take.
 * @param basetype the type that is being initialized.
 * @param n the number of type parameters it has.
 * @param params the parameters this basetype accepts.
 */
void libab_basetype_init(libab_basetype* basetype, int n, 
        const libab_basetype_param params[]);
/**
 * Frees the given basetype.
 * @param basetype the type to free.
 */
void libab_basetype_free(libab_basetype* basetype);

#endif
