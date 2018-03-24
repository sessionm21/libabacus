#ifndef LIBABACUS_TREE_H
#define LIBABACUS_TREE_H

#include "result.h"
#include "parsetype.h"
#include "vec.h"

/**
 * Enum to represent the variant of a tree node.
 */
enum libab_tree_variant_e {
    TREE_NONE,
    TREE_BASE,
    TREE_ID,
    TREE_NUM,
    TREE_OP,
    TREE_RESERVED_OP,
    TREE_PREFIX_OP,
    TREE_POSTFIX_OP,
    TREE_BLOCK,
    TREE_VOID,
    TREE_IF,
    TREE_WHILE,
    TREE_DOWHILE,
    TREE_CALL,
    TREE_FUN,
    TREE_FUN_PARAM,
    TREE_RETURN
};

/**
 * A tree node that has been parsed from the input tokens.
 */
struct libab_tree_s {
    /**
     * The parse type of this node, if applicable.
     */
    libab_parsetype* parse_type;
    /**
     * The variant of tree node.
     */
    enum libab_tree_variant_e variant;
    /**
     * The string value of this tree, if applicable.
     */
    char* string_value;
    /**
     * The int value of this tree, if applicable.
     */
    int int_value;
    /**
     * The children of this tree node. This vector
     * will not be initialized if this tree node
     * type does not usually have children.
     */
    vec children;

    /**
     * The line on which this tree starts.
     */
    size_t line;
    /**
     * The index in the string where this line begins.
     */
    size_t line_from;
    /**
     * The beginning in the string of this tree.
     */
    size_t from;
    /**
     * The index in the string of the next
     * thing that isn't part of this tree.
     */
    size_t to;
};

typedef enum libab_tree_variant_e libab_tree_variant;
typedef struct libab_tree_s libab_tree;

/**
 * Frees the given tree, using its
 * variant as a hint as to what
 * variables need to be freed.
 * @param tree the tree to free.
 */
void libab_tree_free(libab_tree* tree);
/**
 * Determines if the given tree node variant
 * should contain a string.
 * @param var the variant of the tree node.
 * @return true if the tree node variant contains a string.
 */
int libab_tree_has_string(libab_tree_variant var);
/**
 * Determines if the given tree node variant
 * should contain a parse type.
 * @param var the variant of the tree node.
 * @return true if the tree node variant contains a type.
 */
int libab_tree_has_type(libab_tree_variant var);
/**
 * Determines if the given tree node
 * variant contains a scope.
 * @param var the variant of the tree node.
 * @return true if the node variant contains a scope.
 */
int libab_tree_has_scope(libab_tree_variant var);
/**
 * Determines if the given tree node variant
 * should contain a vector.
 * @param var the variant of the tree node.
 * @return true if the tree node variant contains a vector.
 */
int libab_tree_has_vector(libab_tree_variant var);
/**
 * Frees the given tree recursively,
 * deleting the children first and the moving on
 * to the parents. 
 * @param tree the tree to free.
 */
void libab_tree_free_recursive(libab_tree* tree);

#endif
