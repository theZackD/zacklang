#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test arithmetic operations
void test_arithmetic(void)
{
    // Addition
    ASTNode *add = create_binary_expr("+", create_literal("2"), create_literal("3"));
    ComptimeValue *result = evaluate_comptime_expr(add);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 5);
    printf("✓ Addition passed\n");
    free_comptime_value(result);
    free_ast(add);

    // Subtraction
    ASTNode *sub = create_binary_expr("-", create_literal("5"), create_literal("3"));
    result = evaluate_comptime_expr(sub);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 2);
    printf("✓ Subtraction passed\n");
    free_comptime_value(result);
    free_ast(sub);

    // Multiplication
    ASTNode *mul = create_binary_expr("*", create_literal("4"), create_literal("3"));
    result = evaluate_comptime_expr(mul);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 12);
    printf("✓ Multiplication passed\n");
    free_comptime_value(result);
    free_ast(mul);

    // Division
    ASTNode *div = create_binary_expr("/", create_literal("10"), create_literal("2"));
    result = evaluate_comptime_expr(div);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 5);
    printf("✓ Division passed\n");
    free_comptime_value(result);
    free_ast(div);
}

// Test comparison operations
void test_comparison(void)
{
    // Equal
    ASTNode *eq = create_binary_expr("==", create_literal("2"), create_literal("2"));
    ComptimeValue *result = evaluate_comptime_expr(eq);
    assert(result != NULL);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Equality comparison passed\n");
    free_comptime_value(result);
    free_ast(eq);

    // Less than
    ASTNode *lt = create_binary_expr("<", create_literal("2"), create_literal("3"));
    result = evaluate_comptime_expr(lt);
    assert(result != NULL);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Less than comparison passed\n");
    free_comptime_value(result);
    free_ast(lt);
}

// Test logical operations
void test_logical(void)
{
    // AND
    ASTNode *and = create_binary_expr("and", create_literal("true"), create_literal("false"));
    ComptimeValue *result = evaluate_comptime_expr(and);
    assert(result != NULL);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == false);
    printf("✓ Logical AND passed\n");
    free_comptime_value(result);
    free_ast(and);

    // OR
    ASTNode *or = create_binary_expr("or", create_literal("true"), create_literal("false"));
    result = evaluate_comptime_expr(or);
    assert(result != NULL);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Logical OR passed\n");
    free_comptime_value(result);
    free_ast(or);
}

// Test string operations
void test_string_ops(void)
{
    // String concatenation
    ASTNode *concat = create_binary_expr("+", create_literal("\"Hello, \""), create_literal("\"World!\""));
    ComptimeValue *result = evaluate_comptime_expr(concat);
    assert(result != NULL);
    assert(result->type->kind == TYPE_STRING);
    assert(strcmp(result->value.s_val, "Hello, World!") == 0);
    printf("✓ String concatenation passed\n");
    free_comptime_value(result);
    free_ast(concat);

    // String comparison
    ASTNode *str_eq = create_binary_expr("==", create_literal("\"hello\""), create_literal("\"hello\""));
    result = evaluate_comptime_expr(str_eq);
    assert(result != NULL);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ String comparison passed\n");
    free_comptime_value(result);
    free_ast(str_eq);
}

int main()
{
    printf("Running comptime binary operation tests...\n");

    test_arithmetic();
    test_comparison();
    test_logical();
    test_string_ops();

    printf("All binary operation tests passed!\n");
    return 0;
}