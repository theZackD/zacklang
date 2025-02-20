#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test basic literal evaluation
void test_literal_evaluation(void)
{
    // Integer literal
    ASTNode *int_lit = create_literal("42");
    ComptimeValue *int_val = evaluate_comptime_expr(int_lit);
    assert(int_val->type->kind == TYPE_I32);
    assert(int_val->value.i_val == 42);
    printf("✓ Integer literal evaluation passed\n");
    free_comptime_value(int_val);
    free_ast(int_lit);

    // Float literal
    ASTNode *float_lit = create_literal("3.14");
    ComptimeValue *float_val = evaluate_comptime_expr(float_lit);
    assert(float_val->type->kind == TYPE_F64);
    assert(float_val->value.f_val == 3.14);
    printf("✓ Float literal evaluation passed\n");
    free_comptime_value(float_val);
    free_ast(float_lit);

    // Boolean literal
    ASTNode *bool_lit = create_literal("true");
    ComptimeValue *bool_val = evaluate_comptime_expr(bool_lit);
    assert(bool_val->type->kind == TYPE_BOOL);
    assert(bool_val->value.b_val == true);
    printf("✓ Boolean literal evaluation passed\n");
    free_comptime_value(bool_val);
    free_ast(bool_lit);

    // String literal
    ASTNode *str_lit = create_literal("\"hello\"");
    ComptimeValue *str_val = evaluate_comptime_expr(str_lit);
    assert(str_val->type->kind == TYPE_STRING);
    assert(strcmp(str_val->value.s_val, "hello") == 0);
    printf("✓ String literal evaluation passed\n");
    free_comptime_value(str_val);
    free_ast(str_lit);
}

// Test const variable evaluation
void test_const_evaluation(void)
{
    // Create a const variable declaration
    ASTNode *const_decl = create_var_decl(1, "x", "i32", create_literal("42"));

    // Create a reference to the const variable
    ASTNode *const_ref = create_identifier("x");

    // Create a symbol table and add the const variable
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "x", "i32", const_decl);

    // Evaluate the const reference
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(const_ref, table);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 42);
    printf("✓ Basic const evaluation passed\n");
    free_comptime_value(result);

    // Test const in binary expression
    ASTNode *add = create_binary_expr("+", const_ref, create_literal("8"));
    result = evaluate_comptime_expr_with_symbols(add, table);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 50);
    printf("✓ Const in binary expression passed\n");
    free_comptime_value(result);

    // Test const string
    ASTNode *str_decl = create_var_decl(1, "msg", "string", create_literal("\"hello\""));
    add_symbol_with_node(table, "msg", "string", str_decl);
    ASTNode *str_ref = create_identifier("msg");
    result = evaluate_comptime_expr_with_symbols(str_ref, table);
    assert(result != NULL);
    assert(result->type->kind == TYPE_STRING);
    assert(strcmp(result->value.s_val, "hello") == 0);
    printf("✓ Const string evaluation passed\n");
    free_comptime_value(result);

    // Cleanup
    free_ast(add);
    free_ast(str_decl);
    free_ast(const_decl);
    destroy_symbol_table(table);
}

// Test arithmetic operations
void test_arithmetic_operations(void)
{
    // Addition: 2 + 3
    ASTNode *two = create_literal("2");
    ASTNode *three = create_literal("3");
    ASTNode *add = create_binary_expr("+", two, three);
    ComptimeValue *result = evaluate_comptime_expr(add);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 5);
    printf("✓ Addition evaluation passed\n");
    free_comptime_value(result);
    free_ast(add);

    // Mixed type: 2.0 + 3.5
    ASTNode *float_two = create_literal("2.0");
    ASTNode *float_three = create_literal("3.5");
    ASTNode *mixed_add = create_binary_expr("+", float_two, float_three);
    result = evaluate_comptime_expr(mixed_add);
    assert(result->type->kind == TYPE_F64);
    assert(result->value.f_val == 5.5);
    printf("✓ Mixed type addition evaluation passed\n");
    free_comptime_value(result);
    free_ast(mixed_add);

    // Power operator: 2 ** 3
    ASTNode *base = create_literal("2");
    ASTNode *exp = create_literal("3");
    ASTNode *power = create_binary_expr("**", base, exp);
    result = evaluate_comptime_expr(power);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 8);
    printf("✓ Power operator evaluation passed\n");
    free_comptime_value(result);
    free_ast(power);
}

// Test comparison operations
void test_comparison_operations(void)
{
    // Equal: 2 == 2
    ASTNode *two1 = create_literal("2");
    ASTNode *two2 = create_literal("2");
    ASTNode *eq = create_binary_expr("==", two1, two2);
    ComptimeValue *result = evaluate_comptime_expr(eq);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Equality comparison passed\n");
    free_comptime_value(result);
    free_ast(eq);

    // Less than: 2 < 3
    ASTNode *two = create_literal("2");
    ASTNode *three = create_literal("3");
    ASTNode *lt = create_binary_expr("<", two, three);
    result = evaluate_comptime_expr(lt);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Less than comparison passed\n");
    free_comptime_value(result);
    free_ast(lt);
}

// Test logical operations
void test_logical_operations(void)
{
    // AND: true and false
    ASTNode *true_lit1 = create_literal("true");
    ASTNode *false_lit1 = create_literal("false");
    ASTNode *and_expr = create_binary_expr("and", true_lit1, false_lit1);
    ComptimeValue *result = evaluate_comptime_expr(and_expr);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == false);
    printf("✓ Logical AND evaluation passed\n");
    free_comptime_value(result);
    free_ast(and_expr);

    // OR: true or false
    ASTNode *true_lit2 = create_literal("true");
    ASTNode *false_lit2 = create_literal("false");
    ASTNode *or_expr = create_binary_expr("or", true_lit2, false_lit2);
    result = evaluate_comptime_expr(or_expr);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Logical OR evaluation passed\n");
    free_comptime_value(result);
    free_ast(or_expr);

    // NOT: not true
    ASTNode *true_lit3 = create_literal("true");
    ASTNode *not_expr = create_unary_expr("not", true_lit3);
    result = evaluate_comptime_expr(not_expr);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == false);
    printf("✓ Logical NOT evaluation passed\n");
    free_comptime_value(result);
    free_ast(not_expr);
}

// Test error cases
void test_error_cases(void)
{
    // Division by zero
    ASTNode *one = create_literal("1");
    ASTNode *zero = create_literal("0");
    ASTNode *div_zero = create_binary_expr("/", one, zero);
    ComptimeValue *result = evaluate_comptime_expr(div_zero);
    assert(result == NULL);
    printf("✓ Division by zero error handling passed\n");
    free_ast(div_zero);

    // Invalid operation type
    ASTNode *str = create_literal("\"hello\"");
    ASTNode *one_new = create_literal("1");
    ASTNode *invalid_op = create_binary_expr("+", one_new, str);
    result = evaluate_comptime_expr(invalid_op);
    assert(result == NULL);
    printf("✓ Invalid operation type error handling passed\n");
    free_ast(invalid_op);
}

int main()
{
    printf("Running comptime evaluation tests...\n");

    test_literal_evaluation();
    test_const_evaluation();
    test_arithmetic_operations();
    test_comparison_operations();
    test_logical_operations();
    test_error_cases();

    printf("All comptime evaluation tests completed!\n");
    return 0;
}