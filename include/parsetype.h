#ifndef LIBABACUS_PARSETYPE_H
#define LIBABACUS_PARSETYPE_H

#include "result.h"
#include "ref_vec.h"
#include "basetype.h"

#define LIBABACUS_TYPE_F_PARENT (1)
#define LIBABACUS_TYPE_F_PLACE (1 << 1)
#define LIBABACUS_TYPE_F_RESOLVED (1 << 2)

/**
 * A type, either parsed or resolved.
 */
struct libab_parsetype_s {
    /**
     * The variant of the given parse type.
     */
    int variant;
    /**
     * Union that represents the data carried by this type.
     */
    union {
        /**
         * The name of the type that this parse type describes.
         */
        char* name;
        /**
         * The pointer to the base of this type.
         */
        libab_basetype* base;
    } data_u;
    /**
     * A vector of children that this parse type contains.
     * The children are effectively type parameters.
     */
    libab_ref_vec children;
};

typedef struct libab_parsetype_s libab_parsetype;

/**
 * Frees the data associated with this type, ignoring
 * its children.
 * @param type the type to free.
 */
void libab_parsetype_free(libab_parsetype* type);

#endif
