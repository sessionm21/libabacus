#include "gc.h"
#include "refcount.h"
#include <stdlib.h>
#include <stdio.h>
#include <memory.h>
#include "refcount_internal.h"

void libab_gc_list_init(libab_gc_list* list) {
    memset(&list->head_sentinel, 0, sizeof(list->head_sentinel));
    memset(&list->tail_sentinel, 0, sizeof(list->tail_sentinel));
    list->head_sentinel.next = &list->tail_sentinel;
    list->tail_sentinel.prev = &list->head_sentinel;
}

void _gc_count_visit_children(libab_ref_count* ref, libab_visitor_function_ptr func, void* data) {
    if(ref->strong && ref->visit_children) ref->visit_children(ref->data, func, data);
}
void libab_gc_visit_children(libab_ref* ref, libab_visitor_function_ptr func, void* data) {
    if(!ref->null) _gc_count_visit_children(ref->count, func, data);
}

void libab_gc_visit(struct libab_ref_s* ref, libab_visitor_function_ptr visitor, void* data) {
    if(!ref->null) {
        visitor(ref->count, data);
    }
}

void _libab_gc_list_append(libab_gc_list* list,
                               libab_ref_count* node) {
    libab_ref_count* before;
    if(node->next) node->next->prev = node->prev;
    if(node->prev) node->prev->next = node->next;
    before = &list->tail_sentinel;
    node->next = before;
    node->prev = before->prev;
    before->prev->next = node;
    before->prev = node;
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
    if(count->visit_children && count->gc >= 0) {
        count->gc = -1;
        _libab_gc_list_append(list, count);
        _gc_count_visit_children(count, _gc_save, data);
    }
}

void libab_gc_run(libab_gc_list* list) {
    libab_gc_list safe;
    libab_ref_count* head;

    #define ITERATE(CODE) head = list->head_sentinel.next; \
    while(head != &list->tail_sentinel) { \
        libab_ref_count* node = head; \
        CODE;\
        head = head->next; \
    }

    libab_gc_list_init(&safe);
    ITERATE(node->gc = node->weak);
    ITERATE(_gc_count_visit_children(node, _gc_decrement, NULL));
    
    head = list->head_sentinel.next;
    while(head != &list->tail_sentinel) {
        if(head->gc > 0) {
            _gc_save(head, &safe);
            head = &list->head_sentinel;
        }
        head = head->next;
    }

    ITERATE(
        node->weak = -1;
        node->strong = -1;
        if(node->free_func) node->free_func(node->data);
    );

    while ((head = list->head_sentinel.next) != &list->tail_sentinel) {
        head->prev->next = head->next;
        head->next->prev = head->prev;
        free(head);
    }

    if(safe.head_sentinel.next != &safe.tail_sentinel) {
        list->head_sentinel.next = safe.head_sentinel.next;
        list->head_sentinel.next->prev = &list->head_sentinel;
        list->tail_sentinel.prev = safe.tail_sentinel.prev;
        list->tail_sentinel.prev->next = &list->tail_sentinel;
    } else {
        list->head_sentinel.next = &list->tail_sentinel;
        list->tail_sentinel.prev = &list->head_sentinel;
    }
}
