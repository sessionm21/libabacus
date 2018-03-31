#ifndef LIBABACUS_TYPE_H
#define LIBABACUS_TYPE_H

#include "vec.h"
#include "result.h"
#include "refcount.h"
#include "parsetype.h"
#include "table.h"

/**
 * A type in libabacus.
 */
struct libab_type_s {
    /**
     * The name of the type.
     * This is also a unique identifier,
     * used to compare types.
     */
    char* name;
};

typedef struct libab_type_s libab_type;

/**
 * Creates a new instance of a type, with no type parameters, and 
 * the given name.
 * @param ref the reference to populate with the new type.
 * @param name the name to use.
 * @return the result of the creation function, which can fail.
 */
libab_result libab_type_create(libab_ref* ref, const char* name);
/**
 * Constructs a new instance of a type from the given parsetype.
 * @param ref the reference to populate with the new type.
 * @param type the parse type to load.
 * @param table the table to use for looking up types.
 */
libab_result libab_type_from_parsetype(libab_ref* ref, libab_parsetype* type, libab_table* table);

#endif
