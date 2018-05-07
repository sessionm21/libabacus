#include "interpreter.h"
#include "util.h"

void libab_interpreter_init(libab_interpreter* intr,
                            libab_ref* table,
                            libab_ref* type_num,
                            libab_impl* impl) {
    libab_ref_copy(type_num, &intr->type_num);
    libab_ref_copy(table, &intr->base_table);
    intr->impl = impl;
}

struct interpreter_state {
    libab_ref type_num;
    libab_impl* impl;
};

void _interpreter_init(struct interpreter_state* state, libab_interpreter* intr) {
    state->impl = intr->impl;
    libab_ref_copy(&intr->type_num, &state->type_num);
}

void _interpreter_free(struct interpreter_state* state) {
    libab_ref_free(&state->type_num);
}

libab_result _interpreter_create_num_val(struct interpreter_state* state,
                                         libab_ref* into, const char* from) {
    void* data;
    libab_result result = LIBAB_SUCCESS;

    if((data = state->impl->parse_num(from))) {
        result = libab_create_value(into, data, &state->type_num);

        if(result != LIBAB_SUCCESS) {
            ((libab_parsetype*) libab_ref_get(&state->type_num))->data_u.base->free_function(data);
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
    libab_result result;

    _interpreter_init(&state, intr);
    result = _interpreter_run(&state, tree, into, &intr->base_table, 1);
    _interpreter_free(&state);

    return result;
}

void libab_interpreter_free(libab_interpreter* intr) {
    libab_ref_free(&intr->base_table);
    libab_ref_free(&intr->type_num);
}
