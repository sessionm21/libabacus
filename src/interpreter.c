#include "libabacus.h"
#include "util.h"
#include "value.h"

void libab_interpreter_init(libab_interpreter* intr, libab* ab) {
    intr->ab = ab;
}

struct interpreter_state {
    libab* ab;
    libab_table* base_table;
};

void _interpreter_init(struct interpreter_state* state,
                       libab_interpreter* intr) {
    state->ab = intr->ab;
    state->base_table = libab_ref_get(&intr->ab->table);
}

void _interpreter_free(struct interpreter_state* state) {}

libab_result _interpreter_create_num_val(struct interpreter_state* state,
                                         libab_ref* into, const char* from) {
    void* data;
    libab_result result = LIBAB_SUCCESS;

    if ((data = state->ab->impl.parse_num(from))) {
        result = libab_create_value_raw(into, data, &state->ab->type_num);

        if (result != LIBAB_SUCCESS) {
            ((libab_parsetype*)libab_ref_get(&state->ab->type_num))
                ->data_u.base->free_function(data);
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

int _interpreter_type_contains_placeholders(libab_ref* type) {
    size_t index = 0;
    int placeholder;
    libab_ref temp_child;
    libab_parsetype* parsetype = libab_ref_get(type);
    placeholder = (parsetype->variant & LIBABACUS_TYPE_F_PLACE) != 0;
    if(parsetype->variant & LIBABACUS_TYPE_F_PARENT) {
        for(; index < parsetype->children.size && !placeholder; index++) {
            libab_ref_vec_index(&parsetype->children, index, &temp_child);
            placeholder |= _interpreter_type_contains_placeholders(&temp_child);
            libab_ref_free(&temp_child);
        }
    }
    return placeholder;
}

libab_result _interpreter_compare_types(libab_ref* left_type, libab_ref* right_type,
                                        libab_ref_trie* left_params, libab_ref_trie* right_params) {
    libab_result result = LIBAB_SUCCESS;
    int left_placeholder;
    int right_placeholder;
    libab_parsetype* left = libab_ref_get(left_type);
    libab_parsetype* right = libab_ref_get(right_type);
    libab_ref param_type;

    left_placeholder = left->variant & LIBABACUS_TYPE_F_PLACE;
    right_placeholder = right->variant & LIBABACUS_TYPE_F_PLACE;
    if(left_placeholder && right_placeholder) {
        result = LIBAB_AMBIGOUS_TYPE;
    } else {
        if(left_placeholder) {
            const char* name = left->data_u.name;
            libab_ref_trie_get(left_params, name, &param_type);
            left = libab_ref_get(&param_type);
            libab_ref_free(&param_type);
            if(left == NULL) {
                if(!_interpreter_type_contains_placeholders(right_type)) {
                    result = libab_ref_trie_put(left_params, name, right_type);
                } else {
                    result = LIBAB_AMBIGOUS_TYPE;
                }
            }
        } else if(right_placeholder) {
            const char* name = right->data_u.name;
            libab_ref_trie_get(right_params, name, &param_type);
            right = libab_ref_get(&param_type);
            libab_ref_free(&param_type);
            if(right == NULL) {
                if(!_interpreter_type_contains_placeholders(left_type)) {
                    result = libab_ref_trie_put(right_params, name, left_type);
                } else {
                    result = LIBAB_AMBIGOUS_TYPE;
                }
            }
        }

        if(left != NULL && right != NULL) {
            size_t index = 0;
            libab_ref temp_left;
            libab_ref temp_right;
            result = (left->data_u.base == right->data_u.base) ? LIBAB_SUCCESS : LIBAB_MISMATCHED_TYPE;
            if(result == LIBAB_SUCCESS && 
                    (left->variant & LIBABACUS_TYPE_F_PARENT || right->variant & LIBABACUS_TYPE_F_PARENT)) {
                result = (left->variant & right->variant & LIBABACUS_TYPE_F_PARENT) ? LIBAB_SUCCESS : LIBAB_MISMATCHED_TYPE;
                if(result == LIBAB_SUCCESS) {
                    result = (left->children.size == right->children.size) ? LIBAB_SUCCESS : LIBAB_MISMATCHED_TYPE;
                }
                for(; index < left->children.size && result == LIBAB_SUCCESS; index++) {
                    libab_ref_vec_index(&left->children, index, &temp_left);
                    libab_ref_vec_index(&right->children, index, &temp_right);
                    result = _interpreter_compare_types(&temp_left, &temp_right, left_params, right_params);
                    libab_ref_free(&temp_left);
                    libab_ref_free(&temp_right);
                }
            }
        }
    }

    return result;
}

void _free_parsetype(void* parsetype) {
    libab_parsetype_free(parsetype);
    free(parsetype);
}

libab_result _interpreter_copy_resolved_type(libab_ref* type, libab_ref_trie* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_parsetype* copy;
    libab_parsetype* original;

    original = libab_ref_get(type);
    if(original->variant & LIBABACUS_TYPE_F_PLACE) {
        libab_ref_trie_get(params, original->data_u.name, into);
    } else if((copy = malloc(sizeof(*copy)))) {
        size_t index = 0;
        copy->variant = original->variant;
        copy->data_u.base = original->data_u.base;
        if(copy->variant & LIBABACUS_TYPE_F_PARENT) {
            libab_ref child_copy;
            libab_ref temp_child;
            result = libab_ref_vec_init(&copy->children);
            for(; index < original->children.size && result == LIBAB_SUCCESS; index++) {
                libab_ref_vec_index(&original->children, index, &temp_child);
                result = _interpreter_copy_resolved_type(&temp_child, params, &child_copy);
                if(result == LIBAB_SUCCESS) {
                    result = libab_ref_vec_insert(&copy->children, &child_copy);
                }

                if(result != LIBAB_SUCCESS) {
                    libab_parsetype_free(copy);
                }

                libab_ref_free(&child_copy);
                libab_ref_free(&temp_child);
            }

            if(result == LIBAB_SUCCESS) {
                result = libab_ref_new(into, copy, _free_parsetype);
                if(result != LIBAB_SUCCESS) {
                    _free_parsetype(copy);
                }
            }
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

libab_result _interpreter_resolve_type_params(libab_ref* type, libab_ref_trie* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    if(_interpreter_type_contains_placeholders(type)) {
        result = _interpreter_copy_resolved_type(type, params, into);
    } else {
        libab_ref_copy(type, into);
    }
    return result;
}

libab_result _interpreter_check_function(struct interpreter_state* state,
                                        libab_ref* func, libab_ref_vec* params,
                                        libab_ref_vec* types) {
    libab_result result = LIBAB_SUCCESS;
    libab_value* value;
    libab_parsetype* function_type;
    libab_ref_trie function_params;

    libab_ref_trie_init(&function_params);
    value = libab_ref_get(func);
    function_type = libab_ref_get(&value->type);

    if (params->size >= function_type->children.size) {
        result = LIBAB_BAD_CALL;
    } else {
        libab_ref_trie child_params;
        libab_ref left_temp;
        libab_ref right_value_temp;
        libab_ref* right_temp;
        libab_ref produced_type;
        size_t index = 0;
        for(; index < params->size && result == LIBAB_SUCCESS; index++) {
            libab_ref_trie_init(&child_params);

            libab_ref_vec_index(&function_type->children, index, &left_temp);
            libab_ref_vec_index(params, index, &right_value_temp);
            right_temp = &((libab_value*) libab_ref_get(&right_value_temp))->type;
            result = _interpreter_compare_types(&left_temp, 
                    right_temp, &function_params, &child_params);

            if(result == LIBAB_SUCCESS) {
                result = _interpreter_resolve_type_params(right_temp, &child_params, &produced_type);
                if(result != LIBAB_SUCCESS) {
                    libab_ref_free(&produced_type);
                }
            }

            if(result == LIBAB_SUCCESS) {
                result = libab_ref_vec_insert(types, &produced_type);
                libab_ref_free(&produced_type);
            }

            libab_ref_free(&left_temp);
            libab_ref_free(&right_value_temp);

            libab_ref_trie_free(&child_params);
        }
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_vec_clear(types);
    }

    return result;
}

libab_result _interpreter_find_match(struct interpreter_state* state,
                                     libab_function_list* function_values, libab_ref_vec* params, 
                                     libab_ref_vec* new_types, libab_ref* match, int partial) {
    libab_result result = LIBAB_SUCCESS;
    size_t index = 0;
    size_t list_size = libab_function_list_size(function_values);
    int found_match = 0;
    libab_ref_vec temp_new_types;
    libab_ref temp_function_value;
    libab_parsetype* temp_function_type;

    libab_ref_null(match);
    result = libab_ref_vec_init(&temp_new_types);

    for(; index < list_size && result == LIBAB_SUCCESS; index++) {
        libab_function_list_index(function_values, index, &temp_function_value);
        temp_function_type = libab_ref_get(&((libab_value*) libab_ref_get(&temp_function_value))->type);

        if(((temp_function_type->children.size == params->size + 1) && !partial) ||
                ((temp_function_type->children.size > params->size + 1) && partial)) {
            /* We found a function that has the correct number of parameters. */
            result = _interpreter_check_function(state, &temp_function_value, params, &temp_new_types);
            if(result == LIBAB_MISMATCHED_TYPE) {
                /* Mismatch is OK. */
                result = LIBAB_SUCCESS;
            } else if(result == LIBAB_SUCCESS) {
                /* Function matched; now, check for other matching calls.
                 * More than one matching calls = ambigous call. */
                if(!found_match) {
                    /* We haven't found a match previously. Copy data into new_types,
                     * and use new memory for temp list. */
                    found_match = 1;
                    *new_types = temp_new_types;
                    libab_ref_free(match);
                    libab_ref_copy(&temp_function_value, match);
                    result = libab_ref_vec_init(&temp_new_types);
                    if(result != LIBAB_SUCCESS) {
                        libab_ref_vec_free(new_types);
                    }
                } else {
                    /* We've found a match previously. So, new_types are initialized,
                     * and the call is ambigous. Free all data. */
                    libab_ref_vec_free(new_types);
                    libab_ref_vec_free(&temp_new_types);
                    result = LIBAB_AMBIGOUS_CALL;
                }
            } else {
                /* Something bad happened. Free data as best as we can. */
                libab_ref_vec_free(&temp_new_types);
                if(found_match) libab_ref_vec_free(new_types);
            }
        }

        libab_ref_free(&temp_function_value);
    }

    if(result == LIBAB_SUCCESS) {
        libab_ref_vec_free(&temp_new_types);
        if(!found_match)
            libab_ref_null(match);
    } else {
        libab_ref_free(match);
        libab_ref_null(match);
    }

    return result;
}

libab_result _interpreter_cast_param(libab_ref* param, libab_ref* type, libab_ref_vec* into) { 
    libab_result result = LIBAB_SUCCESS;
    libab_value* old_value = libab_ref_get(param);
    libab_ref new_value;

    result = libab_create_value_ref(&new_value, &old_value->data, type);
    if(result == LIBAB_SUCCESS) {
        result = libab_ref_vec_insert(into, &new_value);
    }
    libab_ref_free(&new_value);

    return result;
}

libab_result _interpreter_cast_params(libab_ref_vec* params, libab_ref_vec* new_types, libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    size_t index = 0;
    libab_ref temp_param;
    libab_ref temp_type;

    for(; index < params->size && result == LIBAB_SUCCESS; index++) {
        libab_ref_vec_index(params, index, &temp_param);
        libab_ref_vec_index(new_types, index, &temp_type);

        result = _interpreter_cast_param(&temp_param, &temp_type, into);

        libab_ref_free(&temp_param);
        libab_ref_free(&temp_type);
    }

    return result;
}

libab_result _interpreter_call_tree(libab_tree* tree, libab_ref_vec* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    return result;
}

libab_result _interpreter_call_behavior(libab_behavior* behavior, libab_ref_vec* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    if(behavior->variant == BIMPL_INTERNAL) {
        result = behavior->data_u.internal(params, into);
    } else {
        result = _interpreter_call_tree(behavior->data_u.tree, params, into);
    }
    return result;
}

libab_result _interpreter_perform_call(libab_value* to_call, libab_ref_vec* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_function* function;
    libab_parsetype* function_type;
    size_t new_params;
    function = libab_ref_get(&to_call->data);
    function_type = libab_ref_get(&to_call->type);
    new_params = params->size - function->params.size;
    if(function_type->children.size - new_params == 1) {
        _interpreter_call_behavior(&function->behavior, params, into);
    }
    return result;
}

libab_result _interpreter_cast_and_perform_call(libab_ref* to_call,
                                                libab_ref_vec* params,
                                                libab_ref_vec* new_types,
                                                libab_ref* into) {
    libab_result result;
    libab_ref_vec new_params;
    libab_value* function_value;
    libab_function* function;

    function_value = libab_ref_get(to_call);
    function = libab_ref_get(&function_value->data);
    result = libab_ref_vec_init_copy(&new_params, &function->params);
    if(result == LIBAB_SUCCESS) {
        result = _interpreter_cast_params(params, new_types, &new_params);

        if(result == LIBAB_SUCCESS) {
            result = _interpreter_perform_call(function_value, &new_params, into);
        }

        libab_ref_vec_free(&new_params);
    }
    return result;
}

libab_result _interpreter_call_function_list(struct interpreter_state* state,
                                        libab_function_list* list, libab_ref_vec* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref_vec new_types;
    libab_ref to_call;
    libab_ref_null(into);

    result = _interpreter_find_match(state, list, params, &new_types, &to_call, 0);
    if(result == LIBAB_SUCCESS) {
        if(libab_ref_get(&to_call) == NULL) {
            result = _interpreter_find_match(state, list, params, &new_types, &to_call, 1);
        }
    }

    if(result == LIBAB_SUCCESS && libab_ref_get(&to_call) == NULL) {
        result = LIBAB_BAD_CALL;
    }

    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = _interpreter_cast_and_perform_call(&to_call, params, &new_types, into);
        libab_ref_vec_free(&new_types);
    }

    libab_ref_free(&to_call);

    return result;
}

libab_result _interpreter_call_function(struct interpreter_state* state, libab_ref* function, libab_ref_vec* params, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref_vec temp_new_types;

    libab_ref_null(into);
    result = libab_ref_vec_init(&temp_new_types);
    if(result == LIBAB_SUCCESS) {
        result = _interpreter_check_function(state, function, params, &temp_new_types);

        if(result == LIBAB_SUCCESS) {
            libab_ref_free(into);
            result = _interpreter_cast_and_perform_call(function, params, &temp_new_types, into);
        }

        libab_ref_vec_free(&temp_new_types);
    }

    return result;
}

libab_result _interpreter_run(struct interpreter_state* state, libab_tree* tree,
                              libab_ref* into, libab_ref* scope,
                              int force_scope) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref new_scope;
    int needs_scope = libab_tree_has_scope(tree->variant) || force_scope;

    if (needs_scope) {
        result = libab_create_table(&new_scope, scope);
        scope = &new_scope;
    }

    if (result != LIBAB_SUCCESS) {

    } else if (tree->variant == TREE_BASE || tree->variant == TREE_BLOCK) {
        size_t index = 0;
        libab_ref_null(into);
        while (result == LIBAB_SUCCESS && index < tree->children.size) {
            libab_ref_free(into);
            result = _interpreter_run(state, vec_index(&tree->children, index),
                                      into, scope, 0);
            index++;
        }
    } else if (tree->variant == TREE_NUM) {
        result = _interpreter_create_num_val(state, into, tree->string_value);
    } else if (tree->variant == TREE_VOID) {
        libab_ref_null(into);
    }

    if (needs_scope) {
        libab_ref_free(&new_scope);
    }

    return result;
}

libab_result libab_interpreter_run(libab_interpreter* intr, libab_tree* tree,
                                   libab_ref* into) {
    struct interpreter_state state;
    libab_result result;

    _interpreter_init(&state, intr);
    result = _interpreter_run(&state, tree, into, &state.ab->table, 1);
    _interpreter_free(&state);

    return result;
}

void libab_interpreter_free(libab_interpreter* intr) {}
