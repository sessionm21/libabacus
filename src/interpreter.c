#include "libabacus.h"
#include "util.h"
#include "value.h"
#include "free_functions.h"
#include "reserved.h"

libab_result _create_bool_value(libab* ab, int val, libab_ref* into) {
    libab_ref type_bool;
    int* new_bool;
    libab_result result = LIBAB_SUCCESS;

    libab_get_type_bool(ab, &type_bool);
    new_bool = malloc(sizeof(*new_bool));
    if(new_bool) {
        *new_bool = val;
        result = libab_create_value_raw(ab, into, new_bool, &type_bool);
        if(result != LIBAB_SUCCESS) {
            free(new_bool);
        }
    } else {
        result = LIBAB_MALLOC;
        libab_ref_null(into);
    }
    libab_ref_free(&type_bool);
    return result;
}

libab_result libab_interpreter_init(libab_interpreter* intr, libab* ab) {
    libab_result result;
    libab_ref unit_data;
    intr->ab = ab;
    libab_ref_null(&intr->value_true);
    libab_ref_null(&intr->value_false);

    libab_ref_null(&unit_data);
    result = libab_create_value_ref(ab, &intr->value_unit, &unit_data, &ab->type_unit);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(&intr->value_true);
        result = _create_bool_value(ab, 1, &intr->value_true);
    }
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(&intr->value_false);
        result = _create_bool_value(ab, 0, &intr->value_false);
    }
    libab_ref_free(&unit_data);

    if(result != LIBAB_SUCCESS) {
        libab_ref_free(&intr->value_unit);
        libab_ref_free(&intr->value_true);
        libab_ref_free(&intr->value_false);
    }

    return result;
}

struct interpreter_state {
    libab* ab;
    libab_table* base_table;
};

void _interpreter_init(struct interpreter_state* state,
                       libab_interpreter* intr,
                       libab_ref* scope) {
    state->ab = intr->ab;
    state->base_table = libab_ref_get(scope);
}

void _interpreter_free(struct interpreter_state* state) {}

libab_result _interpreter_create_num_val(struct interpreter_state* state,
                                         libab_ref* into, const char* from) {
    void* data;
    libab_result result = LIBAB_SUCCESS;
    libab_ref_null(into);

    if ((data = state->ab->impl.parse_num(from))) {
        libab_ref_free(into);
        result = libab_create_value_raw(state->ab, into, data, &state->ab->type_num);

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

void _interpreter_search_type_param(libab_ref_trie* params,
                                    libab_ref* scope,
                                    const char* name,
                                    libab_ref* into) {
    libab_ref_trie_get(params, name, into);

    if(libab_ref_get(into) == NULL) {
        libab_table_search_type_param(libab_ref_get(scope), name, into);
    }
}

libab_parsetype* _interpreter_search_type_param_raw(libab_ref_trie* params,
                                                    libab_ref* scope,
                                                    const char* name) {
    libab_ref into;
    libab_parsetype* to_return;
    _interpreter_search_type_param(params, scope, name, &into);
    to_return = libab_ref_get(&into);
    libab_ref_free(&into);
    return to_return;
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
                                        libab_ref_trie* right_params,
                                        libab_ref* scope) {
    libab_result result = LIBAB_SUCCESS;
    int left_placeholder;
    int right_placeholder;
    libab_parsetype* left = libab_ref_get(left_type);
    libab_parsetype* right = libab_ref_get(right_type);

    left_placeholder = left->variant & LIBABACUS_TYPE_F_PLACE;
    right_placeholder = right->variant & LIBABACUS_TYPE_F_PLACE;
    if (left_placeholder && right_placeholder) {
        result = LIBAB_AMBIGOUS_TYPE;
    } else {
        if (left_placeholder) {
            const char* name = left->data_u.name;
            left = _interpreter_search_type_param_raw(left_params, scope, name);
            if (left == NULL) {
                if (!_interpreter_type_contains_placeholders(right_type)) {
                    result = libab_ref_trie_put(left_params, name, right_type);
                } else {
                    result = LIBAB_AMBIGOUS_TYPE;
                }
            }
        } else if (right_placeholder) {
            const char* name = right->data_u.name;
            right = _interpreter_search_type_param_raw(right_params, scope, name);
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
                        &temp_left, &temp_right, left_params, right_params, scope);
                    libab_ref_free(&temp_left);
                    libab_ref_free(&temp_right);
                }
            }
        }
    }

    return result;
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
                                             libab_ref* scope,
                                             libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_parsetype* copy;
    libab_parsetype* original;

    original = libab_ref_get(type);
    if (original->variant & LIBABACUS_TYPE_F_PLACE) {
        _interpreter_search_type_param(params, scope, original->data_u.name, into);
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
                                                         scope, &child_copy);
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
                result = libab_ref_new(into, copy, libab_free_parsetype);
                if (result != LIBAB_SUCCESS) {
                    libab_free_parsetype(copy);
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
                                              libab_ref* scope,
                                              libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    if (_interpreter_type_contains_placeholders(type)) {
        result = _interpreter_copy_resolved_type(type, params, scope, into);
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
                                      libab_ref_vec* types,
                                      libab_ref* scope,
                                      libab_ref_trie* param_map) {
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
                &left_temp, right_temp, &function_params, &child_params, scope);

            if (result == LIBAB_SUCCESS) {
                result = _interpreter_resolve_type_params(
                    right_temp, &child_params, scope, &produced_type);
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

    if(result == LIBAB_SUCCESS) {
        *param_map = function_params;
    } else {
        libab_ref_trie_free(&function_params);
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
                                     libab_ref_vec* new_types, 
                                     libab_ref_trie* param_map,
                                     libab_ref* match,
                                     int partial) {
    libab_result result = LIBAB_SUCCESS;
    size_t index = 0;
    size_t list_size = libab_function_list_size(function_values);
    int found_match = 0;
    libab_ref_trie temp_param_map;
    libab_ref_vec temp_new_types;
    libab_ref temp_function_value;
    libab_value* temp_value;
    libab_parsetype* temp_function_type;
    libab_function* temp_function;

    libab_ref_null(match);
    result = libab_ref_vec_init(&temp_new_types);

    for (; index < list_size && result == LIBAB_SUCCESS; index++) {
        libab_function_list_index(function_values, index, &temp_function_value);
        temp_value = libab_ref_get(&temp_function_value);
        temp_function_type = libab_ref_get(&temp_value->type);
        temp_function = libab_ref_get(&temp_value->data);

        if (((temp_function_type->children.size == params->size + 1) &&
             !partial) ||
            ((temp_function_type->children.size > params->size + 1) &&
             partial)) {
            /* We found a function that has the correct number of parameters. */
            result = _interpreter_check_types(
                &temp_function_type->children, params, &temp_new_types, &temp_function->scope, &temp_param_map);
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
                    *param_map = temp_param_map;
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
                    libab_ref_trie_free(param_map);
                    libab_ref_trie_free(&temp_param_map);
                    result = LIBAB_AMBIGOUS_CALL;
                }
            } else {
                /* Something bad happened. Free data as best as we can. */
                libab_ref_vec_free(&temp_new_types);
                if (found_match) {
                    libab_ref_vec_free(new_types);
                    libab_ref_trie_free(param_map);
                }
            }
        }

        libab_ref_free(&temp_function_value);
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_vec_free(&temp_new_types);
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
libab_result _interpreter_cast_param(libab* ab, libab_ref* param,
                                     libab_ref* type,
                                     libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_value* old_value = libab_ref_get(param);
    libab_ref new_value;

    result = libab_create_value_ref(ab, &new_value, &old_value->data, type);
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
libab_result _interpreter_cast_params(libab* ab,
                                      libab_ref_vec* params,
                                      libab_ref_vec* new_types,
                                      libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    size_t index = 0;
    libab_ref temp_param;
    libab_ref temp_type;

    for (; index < params->size && result == LIBAB_SUCCESS; index++) {
        libab_ref_vec_index(params, index, &temp_param);
        libab_ref_vec_index(new_types, index, &temp_type);

        result = _interpreter_cast_param(ab, &temp_param, &temp_type, into);

        libab_ref_free(&temp_param);
        libab_ref_free(&temp_type);
    }

    return result;
}

libab_result _interpreter_run(struct interpreter_state* state, libab_tree* tree,
                              libab_ref* into, libab_ref* scope,
                              libab_interpreter_scope_mode scope_mode);

/**
 * Calls a tree-based function with the given parameters.
 * @param tree the tree function to call.
 * @param the parameters to give to the function.
 * @param scope the scope used for the call.
 * @param into the reference to store the result into;
 * @return the result of the call.
 */
libab_result _interpreter_call_tree(struct interpreter_state* state,
                                    libab_tree* tree, 
                                    libab_ref_vec* params,
                                    libab_ref* scope,
                                    libab_ref* into) {
    libab_ref new_scope;
    libab_tree* child;
    libab_ref param;
    libab_table* new_scope_raw;
    size_t i;
    libab_result result = libab_create_table(state->ab, &new_scope, scope);

    if(result == LIBAB_SUCCESS) {
        new_scope_raw = libab_ref_get(&new_scope);
        for(i = 0; i < tree->children.size - 1 && result == LIBAB_SUCCESS; i++) {
            child = vec_index(&tree->children, i);
            libab_ref_vec_index(params, i, &param);
            result = libab_put_table_value(new_scope_raw, child->string_value, &param);
            libab_ref_free(&param);
        }
    }

    if(result == LIBAB_SUCCESS) {
        result = _interpreter_run(state, 
                vec_index(&tree->children, tree->children.size - 1),
                into, &new_scope, SCOPE_NONE);
    }

    libab_ref_free(&new_scope);

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
                                        libab_ref* scope,
                                        libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    if (behavior->variant == BIMPL_INTERNAL) {
        result = behavior->data_u.internal(state->ab, scope, params, into);
    } else {
        result = _interpreter_call_tree(state, behavior->data_u.tree, params, scope, into);
    }
    return result;
}

/**
 * Copies a function without copying its partial application parameters.
 * @param function the function to copy.
 * @param into the reference to store the copy into.
 */
libab_result _interpreter_copy_function_basic(libab* ab,
                                              libab_ref* function,
                                              libab_ref* scope,
                                              libab_ref* into) {
    libab_function* func = libab_ref_get(function);
    void (*free_function)(void*) = function->count->free_func;
    return libab_create_function_behavior(ab, into, free_function, &func->behavior, scope);
}

libab_result _interpreter_copy_function_with_params(libab* ab,
                                                    libab_ref* function,
                                                    libab_ref_vec* params,
                                                    libab_ref* scope,
                                                    libab_ref* into) {
    int index = 0;
    libab_ref param;
    libab_function* func;
    libab_result result = _interpreter_copy_function_basic(ab, function, scope, into);
    func = libab_ref_get(into);

    for(; index < params->size && result == LIBAB_SUCCESS; index++) {
        libab_ref_vec_index(params, index, &param);
        result = libab_ref_vec_insert(&func->params, &param);
        libab_ref_free(&param);
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_free(into);
        libab_ref_null(into);
    }

    return result;
}

libab_result _interpreter_copy_type_offset(libab_ref* type,
                                           size_t offset,
                                           libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_parsetype* new_type;
    libab_parsetype* copy_of = libab_ref_get(type);
    if((new_type = malloc(sizeof(*new_type)))) {
        new_type->variant = copy_of->variant;
        new_type->data_u = copy_of->data_u;

        result = libab_ref_vec_init(&new_type->children);
        if(result == LIBAB_SUCCESS) {
            libab_ref child_type_ref;

            for(; offset < copy_of->children.size &&
                    result == LIBAB_SUCCESS; offset++) {
                libab_ref_vec_index(&copy_of->children, offset, &child_type_ref);
                result = libab_ref_vec_insert(&new_type->children, &child_type_ref);
                libab_ref_free(&child_type_ref);
            }

            if(result != LIBAB_SUCCESS) {
                libab_ref_vec_free(&new_type->children);
            }
        }
    } else {
        result = LIBAB_MALLOC;
    }
    
    if(result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, new_type, type->count->free_func);
        if(result != LIBAB_SUCCESS) {
            libab_parsetype_free(new_type);
        }
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }

    return result;
}

libab_result _interpreter_partially_apply(struct interpreter_state* state,
                                          libab_ref* function,
                                          libab_ref_vec* params,
                                          libab_ref* scope,
                                          libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_value* value;
    libab_ref new_type;
    libab_ref new_function;

    value = libab_ref_get(function);
    libab_ref_null(&new_type);
    result = _interpreter_copy_function_with_params(state->ab, &value->data, params, scope, &new_function);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(&new_type);
        result = _interpreter_copy_type_offset(&value->type, params->size, &new_type);
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_create_value_ref(state->ab, into, &new_function, &new_type);
    } else {
        libab_ref_null(into);
    }

    libab_ref_free(&new_function);
    libab_ref_free(&new_type);

    return result;
}

libab_result _interpreter_foreach_insert_param(const libab_ref* param,
                                               const char* key,
                                               va_list args) {
    libab_result result = LIBAB_SUCCESS;
    libab_table_entry* entry;

    if ((entry = malloc(sizeof(*entry)))) {
        entry->variant = ENTRY_TYPE_PARAM;
        libab_ref_copy(param, &entry->data_u.type_param);
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = libab_table_put(libab_ref_get(va_arg(args, libab_ref*)), key, entry);
        if(result != LIBAB_SUCCESS) {
            libab_ref_free(&entry->data_u.type_param);
            free(entry);
        }
    }

    return result;
}

libab_result _interpreter_create_scope(libab* ab,
                                       libab_ref* into,
                                       libab_ref* parent_scope,
                                       libab_ref_trie* param_map) {
    libab_result result = libab_create_table(ab, into, parent_scope);
    if (result == LIBAB_SUCCESS) {
        result = libab_ref_trie_foreach(param_map, _interpreter_foreach_insert_param, into);

        if(result != LIBAB_SUCCESS) {
            libab_ref_free(into);
            libab_ref_null(into);
        }
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
                                                libab_ref* to_call,
                                                libab_ref_vec* params,
                                                libab_ref_trie* param_map,
                                                libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_value* function_value;
    libab_function* function;
    libab_parsetype* function_type;
    libab_ref new_scope;
    size_t new_params;

    function_value = libab_ref_get(to_call);
    function = libab_ref_get(&function_value->data);
    function_type = libab_ref_get(&function_value->type);
    new_params = params->size - function->params.size;

    result = _interpreter_create_scope(state->ab, &new_scope, &function->scope, param_map);

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    } else if (function_type->children.size - new_params == 1) {
        result = _interpreter_call_behavior(state, &function->behavior, params, &new_scope, into);
    } else {
        result = _interpreter_partially_apply(state, to_call, params, &new_scope, into);
    }

    libab_ref_free(&new_scope);

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
    libab_ref* to_call, libab_ref_vec* params,
    libab_ref_vec* new_types,
    libab_ref_trie* param_map,
    libab_ref* into) {
    libab_result result;
    libab_ref_vec new_params;
    libab_value* function_value;
    libab_function* function;

    function_value = libab_ref_get(to_call);
    function = libab_ref_get(&function_value->data);
    result = libab_ref_vec_init_copy(&new_params, &function->params);
    if (result == LIBAB_SUCCESS) {
        result = _interpreter_cast_params(state->ab, params, new_types, &new_params);

        if (result == LIBAB_SUCCESS) {
            result = _interpreter_perform_function_call(state, to_call,
                                                        &new_params, param_map, into);
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
    libab_ref_trie param_map;
    libab_ref_null(into);

    result =
        _interpreter_find_match(list, params, &new_types, &param_map, &to_call, 0);
    if (result == LIBAB_SUCCESS) {
        if (libab_ref_get(&to_call) == NULL) {
            result = _interpreter_find_match(list, params, &new_types, &param_map,
                                             &to_call, 1);
        }
    }

    if (result == LIBAB_SUCCESS && libab_ref_get(&to_call) == NULL) {
        result = LIBAB_BAD_CALL;
    }

    if (result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = _interpreter_cast_and_perform_function_call(state, &to_call, params,
                                                             &new_types, &param_map, into);
        libab_ref_trie_free(&param_map);
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
    libab_ref_trie param_map;
    libab_value* function_value;
    libab_parsetype* function_type;
    libab_function* function_instance;

    function_value = libab_ref_get(function);
    function_type = libab_ref_get(&function_value->type);
    function_instance = libab_ref_get(&function_value->data);
    libab_ref_null(into);

    result = libab_ref_vec_init(&temp_new_types);
    if (result == LIBAB_SUCCESS) {
        result = _interpreter_check_types(&function_type->children,
                                          params, &temp_new_types, &function_instance->scope, &param_map);

        if (result == LIBAB_SUCCESS) {
            libab_ref_free(into);
            result = _interpreter_cast_and_perform_function_call(
                state, function, params, &temp_new_types, &param_map, into);
            libab_ref_trie_free(&param_map);
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

    libab_ref_null(into);
    libab_ref_null(&function_value);
    va_start(args, scope);
    result = libab_ref_vec_init(&params);

    if (result == LIBAB_SUCCESS) {
        /* Get first parameter. */
        result =
            _interpreter_run(state, va_arg(args, libab_tree*), &temp, scope, SCOPE_NORMAL);

        if (result == LIBAB_SUCCESS) {
            result = libab_ref_vec_insert(&params, &temp);
        }

        libab_ref_free(&temp);

        /* If infix, get second parameter. */
        if (result == LIBAB_SUCCESS && to_call->variant == OPERATOR_INFIX) {
            result = _interpreter_run(state, va_arg(args, libab_tree*), &temp,
                                      scope, SCOPE_NORMAL);

            if (result == LIBAB_SUCCESS) {
                result = libab_ref_vec_insert(&params, &temp);
            }

            libab_ref_free(&temp);
        }

        if(result == LIBAB_SUCCESS) {
            libab_ref_free(&function_value);
            result = _interpreter_require_value(scope, to_call->function, &function_value);

        } else {
            state->ab->errormsg = realloc(state->ab->errormsg, strlen(state->ab->errormsg)+strlen(to_call->function)+50);
            sprintf(state->ab->errormsg + strlen(state->ab->errormsg), "unexpected error in function call to %s\n", to_call->function);
            state->ab->error=1;

	      }

        if(result == LIBAB_SUCCESS) {
            libab_ref_free(into);
            result = _interpreter_try_call(state, &function_value, &params, into);
        }

        libab_ref_vec_free(&params);
    }
    va_end(args);
    libab_ref_free(&function_value);

    return result;
}

libab_result _interpreter_run_call_node(struct interpreter_state* state,
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
        result = _interpreter_run(state, child, &param, scope, SCOPE_NORMAL);

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
            scope, SCOPE_NORMAL);
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

int _interpreter_compare_function_param(
        const void* left, const void* right) {
    const libab_tree* data = right;
    return data->variant == TREE_FUN_PARAM;
}

libab_result _interpreter_resolve_and_insert_param(
        libab_parsetype* type, libab_table* scope, libab_ref_vec* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref copy;
    result = libab_resolve_parsetype_copy(type, scope, &copy);
    if(result == LIBAB_SUCCESS) {
        result = libab_ref_vec_insert(into, &copy);
    }
    libab_ref_free(&copy);
    return result;
}

int _interpreter_foreach_insert_function_param(
        void* data, va_list args) {
    libab_tree* tree = data;
    libab_ref_vec* into = va_arg(args, libab_ref_vec*);
    libab_ref* scope = va_arg(args, libab_ref*);
    return _interpreter_resolve_and_insert_param(libab_ref_get(&tree->type),
                                                 libab_ref_get(scope),
                                                 into);
}

libab_result _interpreter_create_function_type(
        struct interpreter_state* state, libab_tree* tree, libab_ref* scope, libab_parsetype** type) {
    libab_result result = LIBAB_SUCCESS;
    libab_basetype* funciton_type = libab_get_basetype_function(state->ab);

    if((*type = malloc(sizeof(**type)))) {
        (*type)->variant = LIBABACUS_TYPE_F_PARENT | LIBABACUS_TYPE_F_RESOLVED;
        (*type)->data_u.base = funciton_type;
        result = libab_ref_vec_init(&(*type)->children);
    } else {
        result = LIBAB_MALLOC;
    }

    if(result == LIBAB_SUCCESS) {
        result = vec_foreach(&tree->children, NULL, _interpreter_compare_function_param, 
                _interpreter_foreach_insert_function_param, &(*type)->children, scope);
        if(result == LIBAB_SUCCESS) {
            result = _interpreter_resolve_and_insert_param(libab_ref_get(&tree->type), 
                    libab_ref_get(scope), &(*type)->children);
        }
        if(result != LIBAB_SUCCESS) {
            libab_ref_vec_free(&(*type)->children);
        }
    }

    if(result != LIBAB_SUCCESS) {
        free(*type);
        *type = NULL;
    }

    return result;
}

libab_result _interpreter_wrap_function_type(
        struct interpreter_state* state, libab_tree* tree, libab_ref* scope, libab_ref* into) {
    libab_parsetype* type;
    libab_result result = _interpreter_create_function_type(state, tree, scope, &type);

    if(result == LIBAB_SUCCESS) {
        result = libab_ref_new(into, type, libab_free_parsetype);
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    }
    return result;
}

libab_result _interpreter_create_function_value(
        struct interpreter_state* state, libab_tree* tree,
        libab_ref* scope, libab_ref* into) {
    libab_result result = LIBAB_SUCCESS;
    libab_ref function, type;

    libab_ref_null(&type);
    libab_ref_null(into);
    result = libab_create_function_tree(state->ab, &function, libab_free_function, tree, scope);

    if(result == LIBAB_SUCCESS) {
        libab_ref_free(&type);
        result = _interpreter_wrap_function_type(state, tree, scope, &type);
    }

    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = libab_create_value_ref(state->ab, into, &function, &type);
    }

    if(result != LIBAB_SUCCESS) {
        libab_ref_free(into);
        libab_ref_null(into);
    }
    libab_ref_free(&function);
    libab_ref_free(&type);

    return result;
}

libab_result _interpreter_expect_boolean(struct interpreter_state* state,
                                         libab_tree* tree, int* into,
                                         libab_ref* scope,
                                         libab_interpreter_scope_mode mode) {
    libab_ref output;
    libab_result result = _interpreter_run(state, tree, &output, scope, mode);
    libab_value* value;
    libab_parsetype* type;
    if(result == LIBAB_SUCCESS) {
        value = libab_ref_get(&output);
        type = libab_ref_get(&value->type);

        if(type->data_u.base != libab_get_basetype_bool(state->ab)) {
            result = LIBAB_BAD_CALL;
        }

        if(result == LIBAB_SUCCESS) {
            *into = *((int*) libab_ref_get(&value->data));
        }
    }
    libab_ref_free(&output);
    return result;
}

libab_result _interpreter_run(struct interpreter_state* state, libab_tree* tree,
                              libab_ref* into, libab_ref* scope,
                              libab_interpreter_scope_mode mode) {
    #define RUN_CHECK(i) _interpreter_expect_boolean(state, \
        vec_index(&tree->children, i), &value, scope, SCOPE_NORMAL)

    libab_result result = LIBAB_SUCCESS;
    libab_ref new_scope;
    int needs_scope = (mode == SCOPE_FORCE) || 
        (mode == SCOPE_NORMAL && libab_tree_has_scope(tree->variant));

    if (needs_scope) {
        result = libab_create_table(state->ab, &new_scope, scope);

        scope = &new_scope;
    }

    if (result != LIBAB_SUCCESS) {
        libab_ref_null(into);
    } else if (tree->variant == TREE_BASE || tree->variant == TREE_BLOCK) {
        size_t index = 0;
        libab_get_unit_value(state->ab, into);
        while (result == LIBAB_SUCCESS && index < tree->children.size) {
            libab_ref_free(into);
            result = _interpreter_run(state, vec_index(&tree->children, index),
                                      into, scope, SCOPE_NORMAL);
            index++;
        }
    } else if (tree->variant == TREE_NUM) {
        result = _interpreter_create_num_val(state, into, tree->string_value);
    } else if (tree->variant == TREE_VOID) {
        libab_get_unit_value(state->ab, into);
    } else if (tree->variant == TREE_ID) {
        result = _interpreter_require_value(scope, tree->string_value, into);
    } else if (tree->variant == TREE_CALL) {
        result = _interpreter_run_call_node(state, tree, into, scope);
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
    } else if (tree->variant == TREE_FUN) {
        libab_ref function;
        result = 
            _interpreter_create_function_value(state, tree, scope, &function);

	if(result != LIBAB_SUCCESS) {
	  puts("FUNCTION_CREATION_FAILED\n");
	  
	}
        if(result == LIBAB_SUCCESS) {
            result = libab_overload_function(state->ab, libab_ref_get(scope),
                    tree->string_value, &function);
            if(result != LIBAB_SUCCESS) {
                libab_ref_free(&function);
                libab_ref_null(&function);
            }
        }

        libab_ref_copy(&function, into);
        libab_ref_free(&function);
    } else if(tree->variant == TREE_RESERVED_OP) {
        const libab_reserved_operator* op = 
            libab_find_reserved_operator(tree->string_value);
        result = op->function(state->ab, scope,
                vec_index(&tree->children, 0),
                vec_index(&tree->children, 1),
                into);
    } else if(tree->variant == TREE_TRUE) {
        libab_get_true_value(state->ab, into);
    } else if(tree->variant == TREE_FALSE) {
        libab_get_false_value(state->ab, into);
    } else if (tree->variant == TREE_IF) {
        int value;
        result = _interpreter_expect_boolean(state,
                vec_index(&tree->children, 0), &value, scope, SCOPE_NORMAL);

        if(result == LIBAB_SUCCESS) {
            result = _interpreter_run(state, vec_index(&tree->children,
                        value ? 1 : 2), into, scope, SCOPE_FORCE);
        } else {
            libab_ref_null(into);
        }
    } else if(tree->variant == TREE_WHILE) {
        int value;
        libab_get_unit_value(state->ab, into);
        while(result == LIBAB_SUCCESS && (result = RUN_CHECK(0)) == LIBAB_SUCCESS && value) {
            libab_ref_free(into);
            result = _interpreter_run(state, vec_index(&tree->children, 1), 
                                      into, scope, SCOPE_FORCE);
        }
    } else if(tree->variant == TREE_DOWHILE) {
        int value;
        libab_get_unit_value(state->ab, into);
        do {
            libab_ref_free(into);
            result = _interpreter_run(state, vec_index(&tree->children, 0),
                                      into, scope, SCOPE_FORCE);
        } while(result == LIBAB_SUCCESS && (result = RUN_CHECK(1)) == LIBAB_SUCCESS && value);
    } else {
        libab_get_unit_value(state->ab, into);
    }

    if (needs_scope) {
        libab_ref_free(&new_scope);
    }

    return result;
}

libab_result libab_interpreter_run(libab_interpreter* intr, libab_tree* tree,
                                   libab_ref* scope,
                                   libab_interpreter_scope_mode mode,
                                   libab_ref* into) {
    struct interpreter_state state;
    libab_result result;

    _interpreter_init(&state, intr, scope);
    result = _interpreter_run(&state, tree, into, scope, mode);
    _interpreter_free(&state);

    return result;
}

libab_result libab_interpreter_call_function(libab_interpreter* intr,
                                            libab_ref* scope,
                                            const char* function,
                                            libab_ref_vec* params,
                                            libab_ref* into) {
    struct interpreter_state state;
    libab_ref function_value;
    libab_result result;

    _interpreter_init(&state, intr, scope);

    libab_ref_null(into);
    result = _interpreter_require_value(scope, 
                                        function, &function_value);
    if(result == LIBAB_SUCCESS) {
        libab_ref_free(into);
        result = _interpreter_try_call(&state, &function_value, params, into);
    }

    _interpreter_free(&state);
    libab_ref_free(&function_value);

    return result;
}

libab_result libab_interpreter_call_value(libab_interpreter* intr,
                                         libab_ref* scope,
                                         libab_ref* function,
                                         libab_ref_vec* params,
                                         libab_ref* into) {
    struct interpreter_state state;
    libab_result result = LIBAB_SUCCESS;

    _interpreter_init(&state, intr, scope);
    result = _interpreter_try_call(&state, function, params, into);
    _interpreter_free(&state);
    
    return result;
}

void libab_interpreter_unit_value(libab_interpreter* intr, libab_ref* into) {
    libab_ref_copy(&intr->value_unit, into);
}

void libab_interpreter_true_value(libab_interpreter* intr, libab_ref* into) {
    libab_ref_copy(&intr->value_true, into);
}

void libab_interpreter_false_value(libab_interpreter* intr, libab_ref* into) {
    libab_ref_copy(&intr->value_false, into);
}

void libab_interpreter_free(libab_interpreter* intr) {
    libab_ref_free(&intr->value_unit);
    libab_ref_free(&intr->value_true);
    libab_ref_free(&intr->value_false);
}
