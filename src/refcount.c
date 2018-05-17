#include "refcount.h"
#include <stdlib.h>
#include <string.h>

libab_result libab_ref_new(libab_ref* ref, void* data,
                           void (*free_func)(void* data)) {
    libab_result result = LIBAB_SUCCESS;
    ref->null = 0;
    ref->strong = 1;
    ref->data = data;
    if ((ref->count = malloc(sizeof(*(ref->count))))) {
        ref->count->strong = ref->count->weak = 1;
        ref->count->free_func = free_func;
    } else {
        result = LIBAB_MALLOC;
    }
    return result;
}

void libab_ref_null(libab_ref* ref) { ref->null = 1; }

void _libab_ref_changed(libab_ref* ref) {
    if (ref->count->strong == 0) {
        ref->count->strong--;
        if (ref->count->free_func) {
            ref->count->free_func(ref->data);
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
        to_return = ref->data;
    }
    return to_return;
}
