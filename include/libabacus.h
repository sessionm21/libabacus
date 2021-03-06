#ifndef LIBABACUS_H
#define LIBABACUS_H

#include "custom.h"
#include "ht.h"
#include "impl.h"
#include "interpreter.h"
#include "lexer.h"
#include "parser.h"
#include "result.h"
#include "table.h"
#include "gc.h"

/**
 * The main struct of libabacus,
 * which essentially holds all the informatiom
 * for the library's state and operation.
 */
struct libab_s {
    /**
     * The lexer used to convert a string
     * to tokens.
     */
    libab_lexer lexer;
    /**
     * The parser used to convert
     * tokens to a tree.
     */
    libab_parser parser;
    /**
     * The interpreter used
     * to run a tree.
     */
    libab_interpreter intr;

    /**
     * The table used to store top-level
     * things like functions and operators.
     */
    libab_ref table;

    /**
     * The allocator used to construct number instances.
     */
    libab_impl impl;

    /**
     * The number type instance.
     */
    libab_ref type_num;
    /**
     * The boolean type instance.
     */
    libab_ref type_bool;
    /**
     * The function list type instance.
     */
    libab_ref type_function_list;
    /**
     * The unit type instance.
     */
    libab_ref type_unit;
    /**
     * List of containers references
     * that should be tracked by the
     * garbage collector for cycles.
     */
    libab_gc_list containers;

    /**
     * Internal; the number basetype. This cannot be a static
     * variable like other built-in types because it has a free function
     * specified by the user.
     */
    libab_basetype basetype_num;
};

typedef struct libab_s libab;

/**
 * Initializes the libabacus struct as well
 * as all its internal structures such as the lexer.
 * @param ab the libabacus instance used to keep state.
 * @param parse_function function used to parse a number.
 * @param free_function function used to free the parsed number.
 * @return the result of the initialization.
 */
libab_result libab_init(libab* ab, void* (*parse_function)(const char*),
                        void (*free_function)(void*));
/**
 * Registers an operator with libabacus.
 * @param ab the libabacus instance to reigster the operator with.
 * @param op the operator string to register.
 * @param precedence the precedence of the operator.
 * @param associativity the associativity of the operator.
 * @param function the function this operator calls.
 * @return the result of the initialization.
 */
libab_result libab_register_operator_infix(libab* ab, const char* op,
                                           int precedence, int associativity,
                                           const char* function);
/**
 * Registers an operation with libabacus that appears
 * before its operand.
 * @param ab the libabacus instance to register the operator with.
 * @param op the operator string to register.
 * @param function the function this operator calls.
 * @return the result of the registration.
 */
libab_result libab_register_operator_prefix(libab* ab, const char* op,
                                            const char* function);
/**
 * Registers an operation with libabacus that appears
 * after its operand.
 * @param ab the libabacus instance to register the operator with.
 * @param op the operator string to register.
 * @param function the function this operator calls.
 * @return the result of the registration.
 */
libab_result libab_register_operator_postfix(libab* ab, const char* op,
                                             const char* function);

/**
 * Registers a function with libabacus.
 * @param ab the libabacus instance used to keep state.
 * @param name the name of the function.
 * @param type the type of this operator.
 * @param func the function that describes the functionality of the function.
 * @return the result of the registration.
 */
libab_result libab_register_function(libab* ab, const char* name,
                                     libab_ref* type, libab_function_ptr func);
/**
 * Registers a base type with abacus.
 * @param ab the libabacus instance used to keep state.
 * @param name the name to register the basetype under.
 * @param basetype the basetype to register.
 * @return the result of the registration.
 */
libab_result libab_register_basetype(libab* ab, const char* name,
                                     libab_basetype* basetype);
/**
 * Constructs and resolves a parse type, similarly to how it's done in the
 * parser.
 * @param ab the libab instance to use for constructing the type.
 * @param into the reference to populate with the given type.
 * @param type the type to parse.
 * @return the result of the operation.
 */
libab_result libab_create_type(libab* ab, libab_ref* into, const char* type);

/**
 * Finds and returns the built-in libabacus number type.
 * @param ab the ab instance for which to return a type.
 * @return the num basetype.
 */
libab_basetype* libab_get_basetype_num(libab* ab);
/**
 * Finds and returns the built-in libabacus boolean type.
 * @param ab the ab instance for which to return a type.
 * @return the boolean basetype.
 */
libab_basetype* libab_get_basetype_bool(libab* ab);
/**
 * Finds and returns the built-in libabacus function type.
 * @param ab the ab instance for which to return a type.
 * @return the function basetype.
 */
libab_basetype* libab_get_basetype_function(libab* ab);
/**
 * Finds and returns the built-in libabacus function list type.
 * @param ab the ab instance for which to return a type.
 * @return the function list basetype.
 */
libab_basetype* libab_get_basetype_function_list(libab* ab);
/**
 * Finds and returns the built-in libabacus unit type.
 * @param ab the ab instance for which to return a type.
 * @return the unit basetype.
 */
libab_basetype* libab_get_basetype_unit(libab* ab);

/**
 * Get the type of a number in this libabacus instance.
 * @param ab the instance to get the type for.
 * @param into the reference to store the type into.
 */
void libab_get_type_num(libab* ab, libab_ref* into);
/**
 * Get the type of a boolean in this libabacus instance.
 * @param ab the instance to get the type for.
 * @param into the reference to store the type into.
 */
void libab_get_type_bool(libab* ab, libab_ref* into);
/**
 * Get the type of the function list in this libabacus instance.
 * @param ab the instance to get the type for.
 * @param into the ference to store the type into.
 */
void libab_get_type_function_list(libab* ab, libab_ref* into);
/**
 * Get the type of the unit in this libabacus instance.
 * @param ab the instance to get the type for.
 * @param into the reference to store the type into.
 */
void libab_get_type_unit(libab* ab, libab_ref* into);

/**
 * Gets the unit value form this libab instance.
 * @param ab the instance to get the unit value from.
 * @param into the reference into which to store the unit value.
 */
void libab_get_unit_value(libab* ab, libab_ref* into);
/**
 * Gets the true value form this libab instance.
 * @param ab the instance to get the true value from.
 * @param into the reference into which to store the true value.
 */
void libab_get_true_value(libab* ab, libab_ref* into);
/**
 * Gets the false value form this libab instance.
 * @param ab the instance to get the false value from.
 * @param into the reference into which to store the false value.
 */
void libab_get_false_value(libab* ab, libab_ref* into);
/**
 * Get the boolean value corresponding to val from this
 * libab instance.
 * @param ab the instance to get the value from.
 * @param val the true or false value to represent.
 * @param into the reference into which to store the value.
 */
void libab_get_bool_value(libab* ab, int val, libab_ref* into);

/**
 * Parses the given piece of code using the given libabacus instance.
 * @param ab the instance to use to parse the code.
 * @param string the source code to parse.
 * @param into the value to store the newly parsed tree into.
 */
libab_result libab_parse(libab* ab, const char* string, libab_tree** into);
/**
 * Executes the given string of code.
 * @param ab the libabacus instance to use for executing code.
 * @param string the string to execute.
 * @param value the reference into which to store the result.
 * @return the result of the computation.
 */
libab_result libab_run(libab* ab, const char* string, libab_ref* value);
/**
 * Runs an already-compiled tree.
 * @param ab the libabacus instance to use for executing code.
 * @param tree the tree the run.
 * @param value the reference into which to store the output.
 * @return the result of the computation.
 */
libab_result libab_run_tree(libab* ab, libab_tree* tree, libab_ref* value);
/**
 * Calls a function with the given name and parameters.
 * @param ab the libabacus instance to use to call the function.
 * @param function the name of the function to call.
 * @param into the reference into which to store the result.
 * @param param_count the number of parameters given to this function.
 * @return the result of the call.
 */
libab_result libab_call_function(libab* ab, const char* function,
                                libab_ref* into,
                                size_t param_count, ...);
/**
 * Calls a string in a given surrounding scope.
 * @param ab the libabacus instance to use to call the function.
 * @param strign the string to run.
 * @param scope the scope to use for calling the string.
 * @param value the reference into which to store the output of the computation.
 * @return the result of the computation.
 */
libab_result libab_run_scoped(libab* ab, const char* string, libab_ref* scope, libab_ref* value);
/**
 * Calls a tree in a given scope.
 * @param ab the libabacus instance to use to call the tree.
 * @param tree the tree to call.
 * @param scope the scope to use for the call.
 * @param value the reference into which to store the output.
 * @return the result of the call.
 */
libab_result libab_run_tree_scoped(libab* ab, libab_tree* tree, libab_ref* scope, libab_ref* value);
/**
 * Calls a function with the given name and parameters using a given scope.
 * @param ab the libabacus instance to use to call the function.
 * @param function the name of the function to call.
 * @param scope the scope in which to perform the call.
 * @param into the reference into which to store the result.
 * @param param_count the number of parameters given to this function.
 * @return the result of the call.
 */
libab_result libab_call_function_scoped(libab* ab, const char* function, libab_ref* scope,
                                       libab_ref* into,
                                       size_t param_count, ...);

/**
 * Releases all the resources allocated by libabacus.
 * @param ab the libabacus instance to release.
 * @return the result of the initialization.
 */
libab_result libab_free(libab* ab);

#endif
