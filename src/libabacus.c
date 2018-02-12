#include "libabacus.h"

libab_result libab_init(libab* ab) {
    libab_table_init(&ab->table);
    return libab_lexer_init(&ab->lexer);
}

libab_result libab_free(libab* ab) {
    libab_table_free(&ab->table);
    return libab_lexer_free(&ab->lexer);
}
