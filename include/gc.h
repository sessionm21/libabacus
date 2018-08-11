#ifndef LIBABACUS_GC_H
#define LIBABACUS_GC_H

struct libab_ref_s;
struct libab_ref_count_s;

/**
 * Struct used to create an interface
 * for a set of objects to be collected.
 */
struct libab_gc_list_s {
    /**
     * The head of the linked list.
     */
    struct libab_ref_count_s* head;
    /**
     * The tail of the linked list.
     */
    struct libab_ref_count_s* tail;
};

typedef struct libab_gc_list_s libab_gc_list;
typedef void (*libab_visitor_function_ptr)(struct libab_ref_count_s* , void*);
typedef void (*libab_visit_function_ptr)(void*, libab_visitor_function_ptr, void*);

void libab_gc_list_init(libab_gc_list* list);
void libab_gc_visit(struct libab_ref_s*, libab_visitor_function_ptr visitor, void*);
void libab_gc_add(struct libab_ref_s* ref,
                      libab_visit_function_ptr visit_children,
                      libab_gc_list* list);
void libab_gc_run(libab_gc_list* list);

#endif
