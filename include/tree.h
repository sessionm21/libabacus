#ifndef LIBABACUS_TREE_H
#define LIBABACUS_TREE_H

#include "libabacus_result.h"
#include "vec.h"

/**
 * Enum to represent the variant of a tree node.
 */
enum libab_tree_variant_e {
    NONE,
    BASE,
    ID,
    STR,
    CHAR,
    NUM,
    BOOL,
    KW,
    OP,
    UNARY_OP,
    BLOCK,
    FUN,
    IF,
    WHILE,
    DOWHILE,
    FOR,
    CALL,
    RETURN
};

/**
 * A tree node that has been parsed from the input tokens.
 */
struct libab_tree_s {
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

#endif
