#ifndef LIBABACUS_PARSETYPE_H
#define LIBABACUS_PARSETYPE_H

#include "result.h"
#include "vec.h"

/**
 * The variant of the given parse type.
 */
enum libab_parsetype_variant_e {
    PT_STRING,
    PT_PLACEHOLDER,
    PT_PARENT
};

/**
 * A parse type.
 * A parse type is a type as it was parsed, not
 * resolved. Effectively, it doesn't have to be valid,
 * or it can contain references to types whose adresses
 * are not yet known. The parse type is recursive,
 * with PT_STRING being the "base case", and PT_PARENT
 * meaning an initialized children vector with the sub-parse types.
 */
struct libab_parsetype_s {
    /**
     * The variant of the given parse type.
     */
    enum libab_parsetype_variant_e variant;
    /**
     * The name of the type that this parse type describes.
     */
    char* name;
    /**
     * A vector of children that this parse type contains.
     * The children are effectively type parameters.
     */
    vec children;
};

typedef enum libab_parsetype_variant_e libab_parsetype_variant;
typedef struct libab_parsetype_s libab_parsetype;

/**
 * Frees the data associated with this type, ignoring
 * its children.
 * @param type the type to free.
 */
void libab_parsetype_free(libab_parsetype* type);
/**
 * Recursively frees the given parse type, calling free
 * on every single type (including the one passed in).
 * @param type the type to free.
 */
void libab_parsetype_free_recursive(libab_parsetype* type);

#endif
