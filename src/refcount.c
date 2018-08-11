#include "refcount.h"
#include <stdlib.h>
#include <string.h>

libab_result libab_ref_new(libab_ref* ref, void* data,
                           void (*free_func)(void* data)) {
    libab_result result = LIBAB_SUCCESS;
    ref->null = 0;
    ref->strong = 1;
    if ((ref->count = malloc(sizeof(*(ref->count))))) {
        ref->count->data = data;
        ref->count->strong = ref->count->weak = 1;
        ref->count->free_func = free_func;
        ref->count->prev = NULL;
        ref->count->next = NULL;
    } else {
        result = LIBAB_MALLOC;
    }
    return result;
}

void libab_ref_null(libab_ref* ref) { ref->null = 1; }

void _ref_gc_count_visit(libab_ref_count* ref, libab_visitor_function_ptr func, void* data) {
    if(ref->strong && ref->visit_children) ref->visit_children(ref->data, func, data);
}
void libab_ref_gc_visit(libab_ref* ref, libab_visitor_function_ptr func, void* data) {
    _ref_gc_count_visit(ref->count, func, data);
}

void _libab_ref_gc_list_append(libab_ref_gc_list* list,
                               libab_ref_count* node) {
    if(node->next) node->next->prev = node->prev;
    if(node->prev) node->prev->next = node->next;

    node->prev = list->tail;
    node->next = NULL;
    if(list->head) {
        list->tail->next = node;
    } else {
        list->head = node;
    }
    list->tail = node;
}
void libab_ref_gc_add(libab_ref* ref,
                      libab_visit_function_ptr visit_children,
                      libab_ref_gc_list* list) {
    ref->count->visit_children = visit_children;
    _libab_ref_gc_list_append(list, ref->count);
}

void _ref_gc_decrement(libab_ref_count* count, void* data) {
    count->gc--;
}
void _ref_gc_save(libab_ref_count* count, void* data) {
    libab_ref_gc_list* list = data;
    if(count->gc != -1) {
        count->gc = -1;
        _libab_ref_gc_list_append(list, count);
        _ref_gc_count_visit(count, _ref_gc_save, data);
    }
}

void libab_ref_gc_run(libab_ref_gc_list* list) {
    libab_ref_gc_list safe;
    libab_ref_count* head;
    size_t count = 0;

    head = list->head;
    while(head) {
        head->gc = head->weak;
    }

    head = list->head;
    while(head) {
        _ref_gc_count_visit(head, _ref_gc_decrement, NULL);
    }

    head = list->head;
    while(head) {
        _ref_gc_count_visit(head, _ref_gc_save, &safe);
    }

    head = list->head;
    while(head) {
        count++;
    }
    printf("Can free %d\n", count);

    list->head = safe.head;
    list->tail = safe.tail;
}

void _libab_ref_changed(libab_ref* ref) {
    if (ref->count->strong == 0) {
        ref->count->strong--;
        if (ref->count->free_func) {
            ref->count->free_func(ref->count->data);
        }
    }
    if (ref->count->weak == 0) {
        free(ref->count);
    }
}

void libab_ref_weaken(libab_ref* ref) {
    if (!ref->null && ref->strong) {
        ref->count->strong--;
        ref->strong = 0;
        _libab_ref_changed(ref);
    }
}

void libab_ref_free(libab_ref* ref) {
    if (!ref->null) {
        ref->count->strong -= ref->strong;
        ref->count->weak--;
        _libab_ref_changed(ref);
    }
}

void libab_ref_copy(const libab_ref* ref, libab_ref* into) {
    if (!ref->null) {
        ref->count->strong++;
        ref->count->weak++;
    }
    memcpy(into, ref, sizeof(*ref));
}

void libab_ref_swap(libab_ref* left, libab_ref* right) {
    libab_ref tmp;
    tmp = *left;
    *left = *right;
    *right = tmp;
}

void libab_ref_data_free(void* data) { free(data); }

void* libab_ref_get(const libab_ref* ref) {
    void* to_return = NULL;
    if (!ref->null && ref->count->strong > 0) {
        to_return = ref->count->data;
    }
    return to_return;
}
