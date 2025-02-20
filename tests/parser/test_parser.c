#include "../../include/parser.h"
#include "../../include/ast.h"
#include "../../include/lexer.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to parse a string and return the AST
static ASTNode *parse_string(const char *code)
{
    TokenArray tokens = tokenize(code);
    Parser parser = init_parser(tokens);
    ASTNode *ast = parse_program(&parser);
    free_token_array(&tokens);
    return ast;
}

// Test variable declarations
void test_var_declarations(void)
{
    // Test basic variable declaration
    ASTNode *ast = parse_string("let x: i32 = 42;");
    assert(ast != NULL);
    assert(ast->type == AST_BLOCK);
    assert(ast->data.block.stmt_count == 1);
    assert(ast->data.block.statements[0]->type == AST_VAR_DECL);
    free_ast(ast);
    printf("✓ Basic variable declaration test passed\n");

    // Test const variable declaration
    ast = parse_string("let const y: f64 = 3.14;");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->data.var_decl.is_const == 1);
    free_ast(ast);
    printf("✓ Const variable declaration test passed\n");
}

// Test function declarations
void test_function_declarations(void)
{
    // Test basic function
    ASTNode *ast = parse_string(
        "fn add(x: i32, y: i32): i32 {\n"
        "    return x + y;\n"
        "}");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->type == AST_FUNC_DEF);
    assert(ast->data.block.statements[0]->data.func_def.param_count == 2);
    free_ast(ast);
    printf("✓ Basic function declaration test passed\n");

    // Test comptime function
    ast = parse_string(
        "comptime fn factorial(n: i32): i32 {\n"
        "    return n;\n"
        "}");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->data.func_def.is_comptime == 1);
    free_ast(ast);
    printf("✓ Comptime function declaration test passed\n");
}

// Test expressions
void test_expressions(void)
{
    // Test binary operations
    ASTNode *ast = parse_string("let x: i32 = 2 + 3 * 4;");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->data.var_decl.initializer->type == AST_BINARY_EXPR);
    free_ast(ast);
    printf("✓ Binary operations test passed\n");

    // Test function calls
    ast = parse_string("print(factorial(5));");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->type == AST_PRINT_STMT);
    assert(ast->data.block.statements[0]->data.print_stmt.expr->type == AST_FUNC_CALL);
    free_ast(ast);
    printf("✓ Function calls test passed\n");
}

// Test control flow statements
void test_control_flow(void)
{
    // Test if statement
    ASTNode *ast = parse_string(
        "if (x > 0) {\n"
        "    print(x);\n"
        "} elif (x < 0) {\n"
        "    print(-x);\n"
        "} else {\n"
        "    print(0);\n"
        "}");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->type == AST_IF_STMT);
    free_ast(ast);
    printf("✓ If statement test passed\n");

    // Test while loop
    ast = parse_string(
        "while (x > 0) {\n"
        "    x = x - 1;\n"
        "}");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->type == AST_WHILE_STMT);
    free_ast(ast);
    printf("✓ While loop test passed\n");

    // Test for loop
    ast = parse_string(
        "for (i in {0:10}) {\n"
        "    print(i);\n"
        "}");
    assert(ast != NULL);
    assert(ast->data.block.statements[0]->type == AST_FOR_STMT);
    free_ast(ast);
    printf("✓ For loop test passed\n");
}

// Test error handling
void test_error_handling(void)
{
    // Test missing semicolon
    TokenArray tokens = tokenize("let x: i32 = 42");
    Parser parser = init_parser(tokens);
    ASTNode *ast = parse_program(&parser);
    assert(parser.had_error == 1);
    if (ast)
        free_ast(ast);
    free_token_array(&tokens);
    printf("✓ Missing semicolon error test passed\n");

    // Test invalid function parameter
    tokens = tokenize("fn add(x: i32,): i32 {}");
    parser = init_parser(tokens);
    ast = parse_program(&parser);
    assert(parser.had_error == 1);
    if (ast)
        free_ast(ast);
    free_token_array(&tokens);
    printf("✓ Invalid function parameter error test passed\n");
}

int main()
{
    printf("Running parser tests...\n");

    test_var_declarations();
    test_function_declarations();
    test_expressions();
    test_control_flow();
    test_error_handling();

    printf("All parser tests completed!\n");
    return 0;
}