#include "custom.h"

void libab_behavior_free(libab_behavior* behavior) {
    libab_ref_free(&behavior->type);
}

void libab_operator_free(libab_operator* op) {
    libab_behavior_free(&op->behavior);
}

void libab_function_free(libab_function* fun) {
    libab_behavior_free(&fun->behavior);
}
