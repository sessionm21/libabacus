#include "gc.h"
#include "refcount.h"
#include <stdlib.h>

void libab_gc_list_init(libab_gc_list* list) {
    list->head = list->tail = NULL;
}

void _gc_count_visit_children(libab_ref_count* ref, libab_visitor_function_ptr func, void* data) {
    if(ref->strong && ref->visit_children) ref->visit_children(ref->data, func, data);
}
void libab_gc_visit_children(libab_ref* ref, libab_visitor_function_ptr func, void* data) {
    _gc_count_visit_children(ref->count, func, data);
}

void _libab_gc_list_append(libab_gc_list* list,
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
void libab_gc_add(libab_ref* ref,
                      libab_visit_function_ptr visit_children,
                      libab_gc_list* list) {
    ref->count->visit_children = visit_children;
    _libab_gc_list_append(list, ref->count);
}

void _gc_decrement(libab_ref_count* count, void* data) {
    if(count->visit_children) count->gc--;
}
void _gc_save(libab_ref_count* count, void* data) {
    libab_gc_list* list = data;
    if(count->visit_children && count->gc != -1) {
        count->gc = -1;
        _libab_gc_list_append(list, count);
        _gc_count_visit_children(count, _gc_save, data);
    }
}

void libab_gc_run(libab_gc_list* list) {
    libab_gc_list safe;
    libab_ref_count* head;
    size_t count = 0;

    head = list->head;
    while(head) {
        head->gc = head->weak;
    }

    head = list->head;
    while(head) {
        _gc_count_visit_children(head, _gc_decrement, NULL);
    }

    head = list->head;
    while(head) {
        _gc_count_visit_children(head, _gc_save, &safe);
    }

    head = list->head;
    while(head) {
        count++;
    }
    printf("Can free %d\n", count);

    list->head = safe.head;
    list->tail = safe.tail;
}
