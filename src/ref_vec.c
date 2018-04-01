#include "ref_vec.h"
#include <stdlib.h>

libab_result libab_ref_vec_init(libab_ref_vec* vec) {
    libab_result result = LIBAB_SUCCESS;
    vec->capacity = LIBABACUS_REF_VEC_INITIAL_SIZE;
    vec->size = 0;
    vec->data = 
        malloc(sizeof(*vec->data) * LIBABACUS_REF_VEC_INITIAL_SIZE);
    if(vec->data == NULL) {
        result = LIBAB_MALLOC;
    }
    return result;
}

libab_result _libab_ref_vec_try_resize(libab_ref_vec* vec) {
    libab_result result = LIBAB_SUCCESS;
    if(vec->size == vec->capacity) {
        libab_ref* new_memory = realloc(vec->data, (vec->capacity *= 2) * sizeof(*vec->data));
        if(new_memory == NULL) {
            free(vec->data);
            result = LIBAB_MALLOC;
        }
    }
    return result;
}

libab_result libab_ref_vec_insert(libab_ref_vec* vec, libab_ref* data) {
    libab_result result = _libab_ref_vec_try_resize(vec);

    if(result == LIBAB_SUCCESS) {
        libab_ref_copy(data, &vec->data[vec->size++]);
    }

    return result;
}

libab_result libab_ref_vec_insert_value(libab_ref_vec* vec, void* data, void (*free_func)(void*)) {
    libab_result result = _libab_ref_vec_try_resize(vec);

    if(result == LIBAB_SUCCESS) {
        result = libab_ref_new(&vec->data[vec->size], data, free_func);
    }

    if(result == LIBAB_SUCCESS) {
        vec->size++;
    }
    
    return result;
}

const libab_ref* libab_ref_vec_index(libab_ref_vec* vec, size_t index) {
    const libab_ref* to_return = NULL;
    if(index < vec->size) {
        to_return = &vec->data[index];
    }
    return to_return;
}

void libab_ref_vec_free(libab_ref_vec* vec) {
    size_t i = 0;
    for(; i < vec->size; i++) {
        libab_ref_free(&vec->data[i]);
    }
    free(vec->data);
}
