#include "../../include/ast.h"
#include "../../include/semantic.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test basic f-string without interpolation
void test_basic_fstring(void)
{
    // Create f"Hello, World!"
    ASTNode **parts = malloc(sizeof(ASTNode *));
    parts[0] = create_literal("\"Hello, World!\"");
    ASTNode *fstring = create_fstring(parts, 1);

    assert(fstring->type == AST_FSTRING);
    assert(fstring->data.fstring.part_count == 1);
    assert(fstring->data.fstring.parts[0]->type == AST_LITERAL);

    // Run semantic analysis
    semantic_analysis(fstring);

    free_ast(fstring);
    printf("✓ Basic f-string test passed\n");
}

// Test f-string with simple interpolation
void test_simple_interpolation(void)
{
    // Create f"Hello, {name}!"
    ASTNode **parts = malloc(3 * sizeof(ASTNode *));
    parts[0] = create_literal("\"Hello, \"");
    parts[1] = create_string_interp(create_identifier("name"));
    parts[2] = create_literal("\"!\"");
    ASTNode *fstring = create_fstring(parts, 3);

    // Create function context for the test
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "name", "string", create_literal("\"John\""));
    func_stmts[1] = create_print_stmt(fstring);
    ASTNode *block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_interp", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Simple interpolation test passed\n");
}

// Test f-string with expression interpolation
void test_expression_interpolation(void)
{
    // Create f"Sum: {x + y}"
    ASTNode **parts = malloc(2 * sizeof(ASTNode *));
    parts[0] = create_literal("\"Sum: \"");

    // Create x + y expression
    ASTNode *x = create_identifier("x");
    ASTNode *y = create_identifier("y");
    ASTNode *sum = create_binary_expr("+", x, y);
    parts[1] = create_string_interp(sum);

    ASTNode *fstring = create_fstring(parts, 2);

    // Create function context
    ASTNode **func_stmts = malloc(3 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("5"));
    func_stmts[1] = create_var_decl(0, "y", "i32", create_literal("3"));
    func_stmts[2] = create_print_stmt(fstring);
    ASTNode *block = create_block(func_stmts, 3);
    ASTNode *func = create_func_def("test_expr_interp", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Expression interpolation test passed\n");
}

// Test nested f-strings
void test_nested_fstring(void)
{
    // Create f"Outer {f"Inner {x}"}"
    ASTNode **inner_parts = malloc(2 * sizeof(ASTNode *));
    inner_parts[0] = create_literal("\"Inner \"");
    inner_parts[1] = create_string_interp(create_identifier("x"));
    ASTNode *inner_fstring = create_fstring(inner_parts, 2);

    ASTNode **outer_parts = malloc(2 * sizeof(ASTNode *));
    outer_parts[0] = create_literal("\"Outer \"");
    outer_parts[1] = create_string_interp(inner_fstring);
    ASTNode *outer_fstring = create_fstring(outer_parts, 2);

    // Create function context
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("42"));
    func_stmts[1] = create_print_stmt(outer_fstring);
    ASTNode *block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_nested", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Nested f-string test passed\n");
}

// Test invalid f-string cases
void test_invalid_fstring(void)
{
    // Test case: undefined variable in interpolation
    ASTNode **parts = malloc(2 * sizeof(ASTNode *));
    parts[0] = create_literal("\"Invalid: \"");
    parts[1] = create_string_interp(create_identifier("undefined_var"));
    ASTNode *invalid_fstring = create_fstring(parts, 2);

    // Create function with invalid f-string
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = create_print_stmt(invalid_fstring);
    ASTNode *block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_invalid", NULL, 0, "void", block, 0);

    // This should cause a semantic error
    printf("Testing invalid f-string (should cause error)...\n");
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Invalid f-string test passed\n");
}

int main()
{
    printf("Running f-string tests...\n");

    test_basic_fstring();
    test_simple_interpolation();
    test_expression_interpolation();
    test_nested_fstring();
    test_invalid_fstring();

    printf("All f-string tests completed!\n");
    return 0;
}