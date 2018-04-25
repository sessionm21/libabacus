#include "interpreter.h"
#include "util.h"

void libab_interpreter_init(libab_interpreter* intr,
                            libab_ref* table,
                            libab_impl* impl) {
    libab_ref_copy(table, &intr->base_table);
    intr->impl = impl;
}

struct interpreter_state {
    libab_ref num_ref;
    libab_impl* impl;
};

libab_result _interpreter_init(struct interpreter_state* state, libab_interpreter* intr) {
    libab_result result = LIBAB_SUCCESS;
    libab_basetype* num_type;
    libab_ref_null(&state->num_ref);
    state->impl = intr->impl;

    num_type = libab_table_search_basetype(libab_ref_get(&intr->base_table), "num");
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

libab_result _interpreter_create_num_val(struct interpreter_state* state,
                                         libab_ref* into, const char* from) {
    void* data;
    libab_result result = LIBAB_SUCCESS;

    if((data = state->impl->parse_num(from))) {
        result = libab_create_value(into, data, &state->num_ref);

        if(result != LIBAB_SUCCESS) {
            ((libab_parsetype*) libab_ref_get(&state->num_ref))->data_u.base->free_function(data);
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
        result = libab_create_table(&new_scope, scope);
        scope = &new_scope;
    }

    if(result != LIBAB_SUCCESS) {

    } else if(tree->variant == TREE_BASE || tree->variant == TREE_BLOCK) {
        size_t index = 0;
        libab_ref_null(into);
        while(result == LIBAB_SUCCESS && index < tree->children.size) {
            libab_ref_free(into);
            result = _interpreter_run(state, vec_index(&tree->children, index), into, scope, 0);
            index++;
        }
    } else if(tree->variant == TREE_NUM) {
        result = _interpreter_create_num_val(state, into, tree->string_value);
    } else if(tree->variant == TREE_VOID) {
        libab_ref_null(into);
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
        result = _interpreter_run(&state, tree, into, &intr->base_table, 1);
        _interpreter_free(&state);
    } else {
        libab_ref_null(into);
    }

    return result;
}

void libab_interpreter_free(libab_interpreter* intr) {
    libab_ref_free(&intr->base_table);
}
