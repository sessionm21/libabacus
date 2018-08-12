#include "refcount_internal.h"
#include <stdlib.h>

void libab_ref_count_changed(libab_ref_count* count) {
    if (count->strong == 0) {
        count->strong--;
        if (count->free_func) {
            count->free_func(count->data);
        }
    }
    if (count->weak == 0) {
        if(count->prev) count->prev->next = count->next;
        if(count->next) count->next->prev = count->prev;
        free(count);
    }
}
