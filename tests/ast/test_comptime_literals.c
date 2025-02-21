#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test integer literal
void test_integer_literal(void)
{
    ASTNode *int_lit = create_literal("42");
    ComptimeValue *int_val = evaluate_comptime_expr(int_lit);
    assert(int_val != NULL);
    assert(int_val->type->kind == TYPE_I32);
    assert(int_val->value.i_val == 42);
    printf("✓ Integer literal evaluation passed\n");
    free_comptime_value(int_val);
    free_ast(int_lit);
}

// Test float literal
void test_float_literal(void)
{
    ASTNode *float_lit = create_literal("3.14");
    ComptimeValue *float_val = evaluate_comptime_expr(float_lit);
    assert(float_val != NULL);
    assert(float_val->type->kind == TYPE_F64);
    assert(float_val->value.f_val == 3.14);
    printf("✓ Float literal evaluation passed\n");
    free_comptime_value(float_val);
    free_ast(float_lit);
}

// Test boolean literal
void test_boolean_literal(void)
{
    // Test true
    ASTNode *true_lit = create_literal("true");
    ComptimeValue *true_val = evaluate_comptime_expr(true_lit);
    assert(true_val != NULL);
    assert(true_val->type->kind == TYPE_BOOL);
    assert(true_val->value.b_val == true);
    printf("✓ Boolean true literal evaluation passed\n");
    free_comptime_value(true_val);
    free_ast(true_lit);

    // Test false
    ASTNode *false_lit = create_literal("false");
    ComptimeValue *false_val = evaluate_comptime_expr(false_lit);
    assert(false_val != NULL);
    assert(false_val->type->kind == TYPE_BOOL);
    assert(false_val->value.b_val == false);
    printf("✓ Boolean false literal evaluation passed\n");
    free_comptime_value(false_val);
    free_ast(false_lit);
}

// Test string literal
void test_string_literal(void)
{
    ASTNode *str_lit = create_literal("\"hello\"");
    ComptimeValue *str_val = evaluate_comptime_expr(str_lit);
    assert(str_val != NULL);
    assert(str_val->type->kind == TYPE_STRING);
    assert(strcmp(str_val->value.s_val, "hello") == 0);
    printf("✓ String literal evaluation passed\n");
    free_comptime_value(str_val);
    free_ast(str_lit);
}

int main()
{
    printf("Running comptime literal evaluation tests...\n");

    test_integer_literal();
    test_float_literal();
    test_boolean_literal();
    test_string_literal();

    printf("All literal evaluation tests passed!\n");
    return 0;
}