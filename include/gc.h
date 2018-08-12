#ifndef LIBABACUS_GC_H
#define LIBABACUS_GC_H

#include "refcount.h"
#include "gc_functions.h"

/**
 * Struct used to create an interface
 * for a set of objects to be collected.
 */
struct libab_gc_list_s {
    /**
     * The head sentinel node.
     */
    struct libab_ref_count_s head_sentinel;
    /**
     * The tail sentinel node.
     */
    struct libab_ref_count_s tail_sentinel;
};

typedef struct libab_gc_list_s libab_gc_list;

/**
 * Initializes a garbage collection tracking list.
 * @param list the list to initialize.
 */
void libab_gc_list_init(libab_gc_list* list);
/**
 * Visits the children of the current node, applying the given function to them.
 * @param ref the reference whose children to visit.
 * @param visitor the function to call for each child.
 * @param data the data to pass to the visitor.
 */
void libab_gc_visit_children(struct libab_ref_s* ref, libab_visitor_function_ptr visitor, void* data);
/**
 * Applies the given visitor function to this reference.
 */
void libab_gc_visit(struct libab_ref_s* ref, libab_visitor_function_ptr visitor, void* data);
/**
 * Adds the given reference to the given garbage collection list,
 * and specifies a function used to reach its children.
 * @param ref the reference whose children to visit.
 * @param visit_children the function used to reach the chilren of this reference.
 * @param list the list to which to add the reference.
 */
void libab_gc_add(struct libab_ref_s* ref,
                      libab_visit_function_ptr visit_children,
                      libab_gc_list* list);
/**
 * Performs garbage collection on a given list of container objects/
 * @param list the list to run collection on.
 */
void libab_gc_run(libab_gc_list* list);

#endif
