#ifndef LIBABACUS_GC_FUNCTIONS_H
#define LIBABACUS_GC_FUNCTIONS_H

struct libab_ref_count_s;
struct libab_ref_s;

typedef void (*libab_visitor_function_ptr)(struct libab_ref_count_s* , void*);
typedef void (*libab_visit_function_ptr)(void*, libab_visitor_function_ptr, void*);

#endif
