#ifndef COMPTIME_H
#define COMPTIME_H

#include "ast.h"
#include "static_types.h"
#include "symbol_table.h"
#include <stdbool.h>
#include <stdint.h>

// Maximum recursion depth for comptime evaluation
#define MAX_RECURSION_DEPTH 1000

// Result of compile-time evaluation.
typedef struct ComptimeValue
{
    Type *type;
    union
    {
        int64_t i_val; // For integer types.
        double f_val;  // For float types.
        bool b_val;    // For booleans.
        char *s_val;   // For strings.
        struct
        { // For structs.
            char *type_name;
            struct ComptimeValue **field_values;
            int field_count;
        } struct_val;
    } value;
} ComptimeValue;

// Create a new comptime value.
ComptimeValue *create_comptime_value(Type *type);

// Free a comptime value.
void free_comptime_value(ComptimeValue *value);

// Convert a comptime value to a string representation.
char *comptime_value_to_string(ComptimeValue *value);

// Evaluate an expression at compile time.
ComptimeValue *evaluate_comptime_expr(ASTNode *expr);

// Evaluate an expression at compile time with a given symbol table.
ComptimeValue *evaluate_comptime_expr_with_symbols(ASTNode *expr, SymbolTable *symbols);

// Evaluate a block of statements at compile time.
ComptimeValue *evaluate_comptime_block(ASTNode *block, SymbolTable *symbols);

// Check if an expression can be evaluated at compile time.
bool is_comptime_expr(ASTNode *expr);

// Evaluate a binary operation at compile time.
ComptimeValue *evaluate_comptime_binary_op(const char *op, ComptimeValue *left, ComptimeValue *right);

// Evaluate a unary operation at compile time.
ComptimeValue *evaluate_comptime_unary_op(const char *op, ComptimeValue *operand);

// Convert a literal to a comptime value.
ComptimeValue *literal_to_comptime_value(const char *literal_value, Type *type);

#endif // COMPTIME_H
