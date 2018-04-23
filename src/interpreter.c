#include "interpreter.h"
#include "util.h"

void libab_interpreter_init(libab_interpreter* intr,
                            libab_table* table, libab_number_impl* impl) {
    intr->base_table = table;
    intr->impl = impl;
}

struct interpreter_state {
    libab_table* base_table;
    libab_number_impl* impl;
    libab_ref num_ref;
};

libab_result _interpreter_init(struct interpreter_state* state, libab_interpreter* intr) {
    libab_result result = LIBAB_SUCCESS;
    libab_basetype* num_type;
    state->base_table = intr->base_table;
    state->impl = intr->impl;
    libab_ref_null(&state->num_ref);

    num_type = libab_table_search_basetype(state->base_table, "num");
    if(num_type != NULL) {
        libab_ref_free(&state->num_ref);
        result = libab_instantiate_basetype(num_type, &state->num_ref, 0);
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_free(&state->num_ref);
    }

    return result;
}

void _interpreter_free(struct interpreter_state* state) {
    libab_ref_free(&state->num_ref);
}

void _free_table(void* data) {
    libab_table_free(data);
    free(data);
}

libab_result _make_scope(libab_ref* into, libab_ref* parent) {
    libab_table* table;
    libab_result result = LIBAB_SUCCESS;
    if((table = malloc(sizeof(*table)))) {
        libab_table_init(table);
        result = libab_ref_new(into, table, _free_table);

        if(result != LIBAB_SUCCESS) {
            _free_table(table);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

libab_result _interpreter_run(struct interpreter_state* state,
                                    libab_tree* tree, libab_ref* into, 
                                    libab_ref* scope, int force_scope) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref new_scope;
    int needs_scope = libab_tree_has_scope(tree->variant) || force_scope;
    
    if(needs_scope) {
        result = _make_scope(&new_scope, scope);
        scope = &new_scope;
    }

    if(result != LIBAB_SUCCESS) {

    } else if(tree->variant == TREE_BASE || tree->variant == TREE_BLOCK) {
        size_t index = 0;
        libab_ref_null(into);
        while(result == LIBAB_SUCCESS && index < tree->children.size) {
            libab_ref_free(into);
            result = _interpreter_run(state, vec_index(&tree->children, index), into, scope, 0);
        }
    }

    if(needs_scope) {
        libab_ref_free(&new_scope);
    }

    return result;
}

libab_result libab_interpreter_run(libab_interpreter* intr,
                                   libab_tree* tree, libab_ref* into) {
    struct interpreter_state state;
    libab_result result = _interpreter_init(&state, intr);

    if(result == LIBAB_SUCCESS) {
        result = _interpreter_run(&state, tree, into, NULL, 1);
        if(result != LIBAB_SUCCESS) {
            _interpreter_free(&state);
        }
    } else {
        libab_ref_null(into);
    }

    return result;
}

void libab_interpreter_free(libab_interpreter* intr) {
    
}
