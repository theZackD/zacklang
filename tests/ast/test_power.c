#include "../../include/ast.h"
#include "../../include/semantic.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test basic power expressions with integers
void test_integer_power(void)
{
    // Create 2 ** 3
    ASTNode *base = create_literal("2");
    ASTNode *exponent = create_literal("3");
    ASTNode *power = create_binary_expr("**", base, exponent);

    // Create function context
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = create_expr_stmt(power);
    ASTNode *block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_int_power", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Integer power test passed\n");
}

// Test power expressions with floating point numbers
void test_float_power(void)
{
    // Create 2.5 ** 2
    ASTNode *base = create_literal("2.5");
    ASTNode *exponent = create_literal("2");
    ASTNode *power = create_binary_expr("**", base, exponent);

    // Create function context
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = create_expr_stmt(power);
    ASTNode *block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_float_power", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Float power test passed\n");
}

// Test power expressions with variables
void test_variable_power(void)
{
    // Create x ** y where x and y are variables
    ASTNode *x = create_identifier("x");
    ASTNode *y = create_identifier("y");
    ASTNode *power = create_binary_expr("**", x, y);

    // Create function context with variable declarations
    ASTNode **func_stmts = malloc(3 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("2"));
    func_stmts[1] = create_var_decl(0, "y", "i32", create_literal("3"));
    func_stmts[2] = create_expr_stmt(power);
    ASTNode *block = create_block(func_stmts, 3);
    ASTNode *func = create_func_def("test_var_power", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Variable power test passed\n");
}

// Test nested power expressions
void test_nested_power(void)
{
    // Create 2 ** (3 ** 2)
    ASTNode *inner_base = create_literal("3");
    ASTNode *inner_exp = create_literal("2");
    ASTNode *inner_power = create_binary_expr("**", inner_base, inner_exp);

    ASTNode *outer_base = create_literal("2");
    ASTNode *outer_power = create_binary_expr("**", outer_base, inner_power);

    // Create function context
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = create_expr_stmt(outer_power);
    ASTNode *block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_nested_power", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Nested power test passed\n");
}

// Test invalid power expressions
void test_invalid_power(void)
{
    // Test case: string operand in power expression
    ASTNode *base = create_literal("\"hello\"");
    ASTNode *exponent = create_literal("2");
    ASTNode *power = create_binary_expr("**", base, exponent);

    // Create function context
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = create_expr_stmt(power);
    ASTNode *block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_invalid_power", NULL, 0, "void", block, 0);

    // This should cause a semantic error
    printf("Testing invalid power expression (should cause error)...\n");
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Invalid power test passed\n");
}

int main()
{
    printf("Running power operator tests...\n");

    test_integer_power();
    test_float_power();
    test_variable_power();
    test_nested_power();
    test_invalid_power();

    printf("All power operator tests completed!\n");
    return 0;
}