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

/**
 * Checks if the given type reference contains any placeholder types.
 * @param type the type to check.
 * @return whether the type has placeholders.
 */
int _interpreter_type_contains_placeholders(libab_ref* type) {
    size_t index = 0;
    int placeholder;
    libab_ref temp_child;
    libab_parsetype* parsetype = libab_ref_get(type);
    placeholder = (parsetype->variant & LIBABACUS_TYPE_F_PLACE) != 0;
    if (parsetype->variant & LIBABACUS_TYPE_F_PARENT) {
        for (; index < parsetype->children.size && !placeholder; index++) {
            libab_ref_vec_index(&parsetype->children, index, &temp_child);
            placeholder |= _interpreter_type_contains_placeholders(&temp_child);
            libab_ref_free(&temp_child);
        }
    }
    return placeholder;
}

/**
 * Compares the two types, filling in any missing type parameters
 * in the respective type tries.
 * @param left_type the left type to compare.
 * @param right_type the right type to compare.
 * @pram left_params the trie into which to store left parameters.
 * @param right_params the trie into which to store right parameters.
 * @param result the result of the operation.
 */
libab_result _interpreter_compare_types(libab_ref* left_type,
                                        libab_ref* right_type,
                                        libab_ref_trie* left_params,
                                        libab_ref_trie* right_params) {
    libab_result result = LIBAB_SUCCESS;
    int left_placeholder;
    int right_placeholder;
    libab_parsetype* left = libab_ref_get(left_type);
    libab_parsetype* right = libab_ref_get(right_type);
    libab_ref param_type;

    left_placeholder = left->variant & LIBABACUS_TYPE_F_PLACE;
    right_placeholder = right->variant & LIBABACUS_TYPE_F_PLACE;
    if (left_placeholder && right_placeholder) {
        result = LIBAB_AMBIGOUS_TYPE;
    } else {
        if (left_placeholder) {
            const char* name = left->data_u.name;
            libab_ref_trie_get(left_params, name, &param_type);
            left = libab_ref_get(&param_type);
            libab_ref_free(&param_type);
            if (left == NULL) {
                if (!_interpreter_type_contains_placeholders(right_type)) {
                    result = libab_ref_trie_put(left_params, name, right_type);
                } else {
                    result = LIBAB_AMBIGOUS_TYPE;
                }
            }
        } else if (right_placeholder) {
            const char* name = right->data_u.name;
            libab_ref_trie_get(right_params, name, &param_type);
            right = libab_ref_get(&param_type);
            libab_ref_free(&param_type);
            if (right == NULL) {
                if (!_interpreter_type_contains_placeholders(left_type)) {
                    result = libab_ref_trie_put(right_params, name, left_type);
                } else {
                    result = LIBAB_AMBIGOUS_TYPE;
                }
            }
        }

        if (left != NULL && right != NULL) {
            size_t index = 0;
            libab_ref temp_left;
            libab_ref temp_right;
            result = (left->data_u.base == right->data_u.base)
                         ? LIBAB_SUCCESS
                         : LIBAB_MISMATCHED_TYPE;
            if (result == LIBAB_SUCCESS &&
                (left->variant & LIBABACUS_TYPE_F_PARENT ||
                 right->variant & LIBABACUS_TYPE_F_PARENT)) {
                result =
                    (left->variant & right->variant & LIBABACUS_TYPE_F_PARENT)
                        ? LIBAB_SUCCESS
                        : LIBAB_MISMATCHED_TYPE;
                if (result == LIBAB_SUCCESS) {
                    result = (left->children.size == right->children.size)
                                 ? LIBAB_SUCCESS
                                 : LIBAB_MISMATCHED_TYPE;
                }
                for (; index < left->children.size && result == LIBAB_SUCCESS;
                     index++) {
                    libab_ref_vec_index(&left->children, index, &temp_left);
                    libab_ref_vec_index(&right->children, index, &temp_right);
                    result = _interpreter_compare_types(
                        &temp_left, &temp_right, left_params, right_params);
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

/**
 * Copies a type, substituting type parameters for their copies
 * from the parameter trie.
 * @param type the type to copy.
 * @param params the type parameter map.
 * @param into the copy destination.
 * @return result the result of the operation.
 */
libab_result _interpreter_copy_resolved_type(libab_ref* type,
                                             libab_ref_trie* params,
                                             libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_parsetype* copy;
    libab_parsetype* original;

    original = libab_ref_get(type);
    if (original->variant & LIBABACUS_TYPE_F_PLACE) {
        libab_ref_trie_get(params, original->data_u.name, into);
    } else if ((copy = malloc(sizeof(*copy)))) {
        size_t index = 0;
        copy->variant = original->variant;
        copy->data_u.base = original->data_u.base;
        if (copy->variant & LIBABACUS_TYPE_F_PARENT) {
            libab_ref child_copy;
            libab_ref temp_child;
            result = libab_ref_vec_init(&copy->children);
            for (; index < original->children.size && result == LIBAB_SUCCESS;
                 index++) {
                libab_ref_vec_index(&original->children, index, &temp_child);
                result = _interpreter_copy_resolved_type(&temp_child, params,
                                                         &child_copy);
                if (result == LIBAB_SUCCESS) {
                    result = libab_ref_vec_insert(&copy->children, &child_copy);
                }

                if (result != LIBAB_SUCCESS) {
                    libab_parsetype_free(copy);
                }

                libab_ref_free(&child_copy);
                libab_ref_free(&temp_child);
            }

            if (result == LIBAB_SUCCESS) {
                result = libab_ref_new(into, copy, _free_parsetype);
                if (result != LIBAB_SUCCESS) {
                    _free_parsetype(copy);
                }
            }
        }
    } else {
        result = LIBAB_MALLOC;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

/**
 * Gets a type of with its template types substituted with types
 * from the parameters trie.
 * @param type the type into which to substitute.
 * @param params the map of param names to their types.
 * @param into the reference into which to store the type.
 */
libab_result _interpreter_resolve_type_params(libab_ref* type,
                                              libab_ref_trie* params,
                                              libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    if (_interpreter_type_contains_placeholders(type)) {
        result = _interpreter_copy_resolved_type(type, params, into);
    } else {
        libab_ref_copy(type, into);
    }
    return result;
}

/**
 * Takes a list of types and a list of parameters that have to match these types,
 * and computes the actual types that these parameters should be given, storing them
 * into a third, pre-initialized vector. On error, clears the output vector.
 * @param reference_types the types to check against.
 * @param params the paramters to check.
 * @param types the destination for the new types.
 */
libab_result _interpreter_check_types(libab_ref_vec* reference_types,
                                      libab_ref_vec* params,
                                      libab_ref_vec* types) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref_trie function_params;

    libab_ref_trie_init(&function_params);

    if (params->size >= reference_types->size) {
        result = LIBAB_BAD_CALL;
    } else {
        libab_ref_trie child_params;
        libab_ref left_temp;
        libab_ref right_value_temp;
        libab_ref* right_temp;
        libab_ref produced_type;
        size_t index = 0;
        for (; index < params->size && result == LIBAB_SUCCESS; index++) {
            libab_ref_trie_init(&child_params);

            libab_ref_vec_index(reference_types, index, &left_temp);
            libab_ref_vec_index(params, index, &right_value_temp);
            right_temp =
                &((libab_value*)libab_ref_get(&right_value_temp))->type;
            result = _interpreter_compare_types(
                &left_temp, right_temp, &function_params, &child_params);

            if (result == LIBAB_SUCCESS) {
                result = _interpreter_resolve_type_params(
                    right_temp, &child_params, &produced_type);
                if (result != LIBAB_SUCCESS) {
                    libab_ref_free(&produced_type);
                }
            }

            if (result == LIBAB_SUCCESS) {
                result = libab_ref_vec_insert(types, &produced_type);
                libab_ref_free(&produced_type);
            }

            libab_ref_free(&left_temp);
            libab_ref_free(&right_value_temp);

            libab_ref_trie_free(&child_params);
        }
    }

    libab_ref_trie_free(&function_params);

    if (result != LIBAB_SUCCESS) {
        libab_ref_vec_clear(types);
    }

    return result;
}

/**
 * Checks through a list of functions, and finds a function that matches the given
 * paramters.
 * @param function_value the list of functions to search.
 * @param parmas the parameters that the function must be able to accept.
 * @param new_types the types that the parameters have to be cast to to be accepted
 * by this function. This variable is only initialized if a match is found.
 * @param match the reference into which to store the function value to call, if any.
 * @return the result of the operation.
 */
libab_result _interpreter_find_match(libab_function_list* function_values,
                                     libab_ref_vec* params,
                                     libab_ref_vec* new_types, libab_ref* match,
                                     int partial) {
    libab_result result = LIBAB_SUCCESS;
    size_t index = 0;
    size_t list_size = libab_function_list_size(function_values);
    int found_match = 0;
    libab_ref_vec temp_new_types;
    libab_ref temp_function_value;
    libab_parsetype* temp_function_type;

    libab_ref_null(match);
    result = libab_ref_vec_init(&temp_new_types);

    for (; index < list_size && result == LIBAB_SUCCESS; index++) {
        libab_function_list_index(function_values, index, &temp_function_value);
        temp_function_type = libab_ref_get(
            &((libab_value*)libab_ref_get(&temp_function_value))->type);

        if (((temp_function_type->children.size == params->size + 1) &&
             !partial) ||
            ((temp_function_type->children.size > params->size + 1) &&
             partial)) {
            /* We found a function that has the correct number of parameters. */
            result = _interpreter_check_types(
                &temp_function_type->children, params, &temp_new_types);
            if (result == LIBAB_MISMATCHED_TYPE) {
                /* Mismatch is OK. */
                result = LIBAB_SUCCESS;
            } else if (result == LIBAB_SUCCESS) {
                /* Function matched; now, check for other matching calls.
                 * More than one matching calls = ambigous call. */
                if (!found_match) {
                    /* We haven't found a match previously. Copy data into
                     * new_types, and use new memory for temp list. */
                    found_match = 1;
                    *new_types = temp_new_types;
                    libab_ref_free(match);
                    libab_ref_copy(&temp_function_value, match);
                    result = libab_ref_vec_init(&temp_new_types);
                    if (result != LIBAB_SUCCESS) {
                        libab_ref_vec_free(new_types);
                    }
                } else {
                    /* We've found a match previously. So, new_types are
                     * initialized, and the call is ambigous. Free all data. */
                    libab_ref_vec_free(new_types);
                    libab_ref_vec_free(&temp_new_types);
                    result = LIBAB_AMBIGOUS_CALL;
                }
            } else {
                /* Something bad happened. Free data as best as we can. */
                libab_ref_vec_free(&temp_new_types);
                if (found_match)
                    libab_ref_vec_free(new_types);
            }
        }

        libab_ref_free(&temp_function_value);
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_vec_free(&temp_new_types);
        if (!found_match)
            libab_ref_null(match);
    } else {
        libab_ref_free(match);
        libab_ref_null(match);
    }

    return result;
}

/**
 * Create a new value with the same data and a new type.
 * @param param the value to cast.
 * @param type the new type.
 * @param into the reference into which to store the new value.
 */
libab_result _interpreter_cast_param(libab_ref* param, libab_ref* type,
                                     libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_value* old_value = libab_ref_get(param);
    libab_ref new_value;

    result = libab_create_value_ref(&new_value, &old_value->data, type);
    if (result == LIBAB_SUCCESS) {
        result = libab_ref_vec_insert(into, &new_value);
    }
    libab_ref_free(&new_value);

    return result;
}

/**
 * Casts a list of parameters to the given list of types.
 * @param params the parameters to cast.
 * @param new_types the types to cast to.
 * @param into the pre-initialized vector to store the new values into.
 * @return the result of any allocations.
 */
libab_result _interpreter_cast_params(libab_ref_vec* params,
                                      libab_ref_vec* new_types,
                                      libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    size_t index = 0;
    libab_ref temp_param;
    libab_ref temp_type;

    for (; index < params->size && result == LIBAB_SUCCESS; index++) {
        libab_ref_vec_index(params, index, &temp_param);
        libab_ref_vec_index(new_types, index, &temp_type);

        result = _interpreter_cast_param(&temp_param, &temp_type, into);

        libab_ref_free(&temp_param);
        libab_ref_free(&temp_type);
    }

    return result;
}

/**
 * Calls a tree-based function with the given parameters.
 * @param tree the tree function to call.
 * @param the parameters to give to the function.
 * @param into the reference to store the result into;
 * @return the result of the call.
 */
libab_result _interpreter_call_tree(libab_tree* tree, libab_ref_vec* params,
                                    libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    return result;
}

/**
 * Calls the given behavior with the given parameters.
 * @param state the state in which to perform the call.
 * @param behavior the behavior to clal.
 * @param params the parameters to give to the behavior.
 * @param into the reference into which to store the result of the call.
 * @return libab_result the result of the call.
 */
libab_result _interpreter_call_behavior(struct interpreter_state* state,
                                        libab_behavior* behavior,
                                        libab_ref_vec* params,
                                        libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    if (behavior->variant == BIMPL_INTERNAL) {
        result = behavior->data_u.internal(state->ab, params, into);
    } else {
        result = _interpreter_call_tree(behavior->data_u.tree, params, into);
    }
    return result;
}

/**
 * Calls a function with the given, compatible paramters.
 * @param state the state to use to call the function.
 * @param to_call the function value to call.
 * @param params the parameters to pass to the function.
 * @param into the reference into which to store the result.
 * @return the result of the call.
 */
libab_result _interpreter_perform_function_call(struct interpreter_state* state,
                                                libab_value* to_call,
                                                libab_ref_vec* params,
                                                libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_function* function;
    libab_parsetype* function_type;
    size_t new_params;
    function = libab_ref_get(&to_call->data);
    function_type = libab_ref_get(&to_call->type);
    new_params = params->size - function->params.size;
    if (function_type->children.size - new_params == 1) {
        _interpreter_call_behavior(state, &function->behavior, params, into);
    }
    return result;
}

/**
 * Casts the parameters to the given new types, then calls a function.
 * @param state the state to use to call.
 * @param to_call the function to call.
 * @param params the parameters to cast then pass to the function.
 * @param new_types the types to cast the params to.
 * @param into the reference to store the result of the call into.
 * @return the result of the call.
 */
libab_result _interpreter_cast_and_perform_function_call(
    struct interpreter_state* state,
    libab_ref* to_call, libab_ref_vec* params, libab_ref_vec* new_types,
    libab_ref* into) {
    libab_result result;
    libab_ref_vec new_params;
    libab_value* function_value;
    libab_function* function;

    function_value = libab_ref_get(to_call);
    function = libab_ref_get(&function_value->data);
    result = libab_ref_vec_init_copy(&new_params, &function->params);
    if (result == LIBAB_SUCCESS) {
        result = _interpreter_cast_params(params, new_types, &new_params);

        if (result == LIBAB_SUCCESS) {
            result = _interpreter_perform_function_call(state, function_value,
                                                        &new_params, into);
        }

        libab_ref_vec_free(&new_params);
    }
    return result;
}

/**
 * Calls a function list with the given parameters.
 * @param state the state to use to call the list.
 * @param list the list to call.
 * @param params the parameters to pass to the function list.
 * @param into the reference into which to store the result of the call.
 * @return the result of the call.
 */
libab_result _interpreter_call_function_list(struct interpreter_state* state,
                                             libab_function_list* list,
                                             libab_ref_vec* params,
                                             libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref_vec new_types;
    libab_ref to_call;
    libab_ref_null(into);

    result =
        _interpreter_find_match(list, params, &new_types, &to_call, 0);
    if (result == LIBAB_SUCCESS) {
        if (libab_ref_get(&to_call) == NULL) {
            result = _interpreter_find_match(list, params, &new_types,
                                             &to_call, 1);
        }
    }

    if (result == LIBAB_SUCCESS && libab_ref_get(&to_call) == NULL) {
        result = LIBAB_BAD_CALL;
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = _interpreter_cast_and_perform_function_call(state, &to_call, params,
                                                             &new_types, into);
        libab_ref_vec_free(&new_types);
    }

    libab_ref_free(&to_call);

    return result;
}

/**
 * Calls a function with the given parameters.
 * @param state the state to use to call the.
 * @param function the function to call.
 * @param params the parameters to pass to the function.
 * @param into the reference into which to store the result of the call.
 * @return the result of the call.
 */
libab_result _interpreter_call_function(struct interpreter_state* state,
                                        libab_ref* function,
                                        libab_ref_vec* params,
                                        libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref_vec temp_new_types;
    libab_value* function_value;
    libab_parsetype* function_type;

    function_value = libab_ref_get(function);
    function_type = libab_ref_get(&function_value->type);
    libab_ref_null(into);

    result = libab_ref_vec_init(&temp_new_types);
    if (result == LIBAB_SUCCESS) {
        result = _interpreter_check_types(&function_type->children,
                                          params, &temp_new_types);

        if (result == LIBAB_SUCCESS) {
            libab_ref_free(into);
            result = _interpreter_cast_and_perform_function_call(
                state, function, params, &temp_new_types, into);
        }

        libab_ref_vec_free(&temp_new_types);
    }

    return result;
}

/**
 * Attempts to call a value of unknown type.
 * @param state the state in which to run the code.
 * @param value the value which is being called.
 * @param params the parameters given to the value.
 * @param into the reference into which to store the output of the call.
 * @return the result of the call.
 */
libab_result _interpreter_try_call(struct interpreter_state* state,
                                   libab_ref* value, libab_ref_vec* params,
                                   libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_value* callee_value;
    libab_parsetype* callee_type;
    libab_basetype* callee_basetype;
    callee_value = libab_ref_get(value);
    callee_type = libab_ref_get(&callee_value->type);
    callee_basetype = callee_type->data_u.base;

    if (callee_basetype == libab_get_basetype_function_list(state->ab)) {
        result = _interpreter_call_function_list(
            state, libab_ref_get(&callee_value->data), params, into);
    } else if (callee_basetype == libab_get_basetype_function(state->ab)) {
        result = _interpreter_call_function(state, value, params, into);
    } else {
        libab_ref_null(into);
        result = LIBAB_BAD_CALL;
    }

    return result;
}

libab_result _interpreter_run(struct interpreter_state* state, libab_tree* tree,
                              libab_ref* into, libab_ref* scope,
                              int force_scope);

libab_result _interpreter_require_value(libab_ref* scope, const char* name,
                                        libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_search_value(libab_ref_get(scope), name, into);
    if(libab_ref_get(into) == NULL) {
        result = LIBAB_UNEXPECTED;
    }
    return result;
}

libab_result _interpreter_call_operator(struct interpreter_state* state,
                                        libab_operator* to_call,
                                        libab_ref* into, libab_ref* scope,
                                        ...) {
    va_list args;
    libab_result result = LIBAB_SUCCESS;
    libab_ref function_value;
    libab_ref_vec params;
    libab_ref temp;

    libab_ref_null(&function_value);
    va_start(args, scope);
    result = libab_ref_vec_init(&params);

    if (result == LIBAB_SUCCESS) {
        /* Get first parameter. */
        result =
            _interpreter_run(state, va_arg(args, libab_tree*), &temp, scope, 0);

        if (result == LIBAB_SUCCESS) {
            result = libab_ref_vec_insert(&params, &temp);
        }

        libab_ref_free(&temp);

        /* If infix, get second parameter. */
        if (result == LIBAB_SUCCESS && to_call->variant == OPERATOR_INFIX) {
            result = _interpreter_run(state, va_arg(args, libab_tree*), &temp,
                                      scope, 0);

            if (result == LIBAB_SUCCESS) {
                result = libab_ref_vec_insert(&params, &temp);
            }

            libab_ref_free(&temp);
        }

        if(result == LIBAB_SUCCESS) {
            libab_ref_free(&function_value);
            result = _interpreter_require_value(scope, to_call->function, &function_value);
        }

        if(result == LIBAB_SUCCESS) {
            result = _interpreter_try_call(state, &function_value, &params, into);
        }

        libab_ref_vec_free(&params);
    }
    va_end(args);
    libab_ref_free(&function_value);

    return result;
}

libab_result _interpreter_run_function_node(struct interpreter_state* state,
                                            libab_tree* tree, libab_ref* into,
                                            libab_ref* scope) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref param;
    libab_ref callee;
    libab_ref_vec params;
    size_t count = 0;
    void* child;

    libab_ref_null(&param);
    result = libab_ref_vec_init(&params);
    for (; count < tree->children.size - 1 && result == LIBAB_SUCCESS;
         count++) {
        libab_ref_free(&param);
        child = vec_index(&tree->children, count);
        result = _interpreter_run(state, child, &param, scope, 0);

        if (result == LIBAB_SUCCESS) {
            result = libab_ref_vec_insert(&params, &param);
        }

        if (result != LIBAB_SUCCESS) {
            libab_ref_vec_free(&params);
        }
    }
    libab_ref_free(&param);

    if (result == LIBAB_SUCCESS) {
        result = _interpreter_run(
            state, vec_index(&tree->children, tree->children.size - 1), &callee,
            scope, 0);
        if (result != LIBAB_SUCCESS) {
            libab_ref_vec_free(&params);
        }
    }

    if (result == LIBAB_SUCCESS) {
        result = _interpreter_try_call(state, &callee, &params, into);
        libab_ref_free(&callee);
        libab_ref_vec_free(&params);
    } else {
        libab_ref_null(into);
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
    } else if (tree->variant == TREE_ID) {
        result = _interpreter_require_value(scope, tree->string_value, into);
    } else if (tree->variant == TREE_CALL) {
        result = _interpreter_run_function_node(state, tree, into, scope);
    } else if (tree->variant == TREE_OP) {
        libab_operator* to_call = libab_table_search_operator(
            libab_ref_get(scope), tree->string_value, OPERATOR_INFIX);
        result = _interpreter_call_operator(state, to_call, into, scope,
                                            vec_index(&tree->children, 0),
                                            vec_index(&tree->children, 1));
    } else if (tree->variant == TREE_PREFIX_OP) {
        libab_operator* to_call = libab_table_search_operator(
            libab_ref_get(scope), tree->string_value, OPERATOR_PREFIX);
        result = _interpreter_call_operator(state, to_call, into, scope,
                                            vec_index(&tree->children, 0));
    } else if (tree->variant == TREE_POSTFIX_OP) {
        libab_operator* to_call = libab_table_search_operator(
            libab_ref_get(scope), tree->string_value, OPERATOR_POSTFIX);
        result = _interpreter_call_operator(state, to_call, into, scope,
                                            vec_index(&tree->children, 0));
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

libab_result libab_interpreter_run_function(libab_interpreter* intr,
                                            const char* function,
                                            libab_ref_vec* params,
                                            libab_ref* into) {
    struct interpreter_state state;
    libab_ref function_value;
    libab_result result;

    _interpreter_init(&state, intr);

    libab_ref_null(into);
    result = _interpreter_require_value(&state.ab->table, 
                                        function, &function_value);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        _interpreter_try_call(&state, &function_value, params, into);
    }

    _interpreter_free(&state);

    return result;
}

void libab_interpreter_free(libab_interpreter* intr) {}
