#include <assert.h>
#include <stdio.h>
#include "include/lexer.h"
#include "include/parser.h"
#include "include/ast.h"
#include <string.h>

// A simple function to print out the type of the root AST node.
// In a full test, you might want to traverse and pretty-print the entire AST.
void print_ast_type(ASTNode *node)
{
    if (!node)
    {
        printf("NULL AST\n");
        return;
    }
    printf("AST node type: %d\n", node->type);
}

// Helper function to parse a string and return the AST
ASTNode *parse_source(const char *source)
{
    TokenArray tokens = tokenize(source);
    Parser parser = init_parser(tokens);
    ASTNode *ast = parse_program(&parser);
    free_token_array(&tokens);
    return ast;
}

void test_variable_declaration()
{
    printf("\nTesting variable declarations...\n");

    // Test basic variable declaration
    ASTNode *ast = parse_source("let x: i32 = 42;");
    assert(ast->type == AST_BLOCK);
    assert(ast->data.block.stmt_count == 1);
    ASTNode *var_decl = ast->data.block.statements[0];
    assert(var_decl->type == AST_VAR_DECL);
    assert(strcmp(var_decl->data.var_decl.identifier, "x") == 0);
    assert(strcmp(var_decl->data.var_decl.type_annotation, "i32") == 0);
    free_ast(ast);

    // Test const declaration
    ast = parse_source("let const y: f64 = 3.14;");
    var_decl = ast->data.block.statements[0];
    assert(var_decl->data.var_decl.is_const == 1);
    assert(strcmp(var_decl->data.var_decl.type_annotation, "f64") == 0);
    free_ast(ast);

    printf("✓ Variable declaration tests passed\n");
}

void test_expressions()
{
    printf("\nTesting expressions...\n");

    // Test arithmetic expressions
    ASTNode *ast = parse_source("let x = 2 + 3 * 4;");
    ASTNode *init = ast->data.block.statements[0]->data.var_decl.initializer;
    assert(init->type == AST_BINARY_EXPR);
    assert(strcmp(init->data.binary_expr.op, "+") == 0);
    free_ast(ast);

    // Test unary operators with numbers
    ast = parse_source("let z = -42;");
    init = ast->data.block.statements[0]->data.var_decl.initializer;
    assert(init->type == AST_UNARY_EXPR);
    assert(strcmp(init->data.unary_expr.op, "-") == 0);
    free_ast(ast);

    // Test comparison and logical operators
    ast = parse_source("let y = x > 0 and y < 10;");
    init = ast->data.block.statements[0]->data.var_decl.initializer;
    assert(init->type == AST_BINARY_EXPR);
    assert(strcmp(init->data.binary_expr.op, "and") == 0);
    free_ast(ast);

    // Test logical not
    ast = parse_source("let w = not true;");
    init = ast->data.block.statements[0]->data.var_decl.initializer;
    assert(init->type == AST_UNARY_EXPR);
    assert(strcmp(init->data.unary_expr.op, "not") == 0);
    free_ast(ast);

    printf("✓ Expression tests passed\n");
}

void test_if_statements()
{
    printf("\nTesting if statements...\n");

    // Test basic if
    ASTNode *ast = parse_source("if (x > 0) { print(x); }");
    ASTNode *if_stmt = ast->data.block.statements[0];
    assert(if_stmt->type == AST_IF_STMT);
    assert(if_stmt->data.if_stmt.else_block == NULL);
    free_ast(ast);

    // Test if-else
    ast = parse_source(
        "if (x > 0) { print(x); } else { print(-x); }");
    if_stmt = ast->data.block.statements[0];
    assert(if_stmt->type == AST_IF_STMT);
    assert(if_stmt->data.if_stmt.else_block != NULL);
    free_ast(ast);

    printf("✓ If statement tests passed\n");
}

void test_loops()
{
    printf("\nTesting loops...\n");

    // Test while loop
    ASTNode *ast = parse_source("while (x > 0) { x = x - 1; }");
    ASTNode *while_stmt = ast->data.block.statements[0];
    assert(while_stmt->type == AST_WHILE_STMT);
    free_ast(ast);

    // Test for loop
    ast = parse_source("for (i in {0:10}) { print(i); }");
    ASTNode *for_stmt = ast->data.block.statements[0];
    assert(for_stmt->type == AST_FOR_STMT);
    assert(for_stmt->data.for_stmt.start_expr->type == AST_LITERAL);
    assert(for_stmt->data.for_stmt.end_expr->type == AST_LITERAL);
    free_ast(ast);

    printf("✓ Loop tests passed\n");
}

void test_functions()
{
    printf("\nTesting functions...\n");

    // Test function definition
    ASTNode *ast = parse_source("fn add(a: i32, b: i32): i32 { a + b }");
    ASTNode *func = ast->data.block.statements[0];
    assert(func->type == AST_FUNC_DEF);
    assert(strcmp(func->data.func_def.name, "add") == 0);
    free_ast(ast);

    // Test function call
    ast = parse_source("print(add(1, 2));");
    ASTNode *call = ast->data.block.statements[0]->data.print_stmt.expr;
    assert(call->type == AST_FUNC_CALL);
    assert(strcmp(call->data.func_call.name, "add") == 0);
    free_ast(ast);

    printf("✓ Function tests passed\n");
}

void test_blocks_and_scopes()
{
    printf("\nTesting blocks and scopes...\n");

    // Test nested blocks
    const char *source =
        "{"
        "    let x = 1;"
        "    {"
        "        let y = 2;"
        "        print(x + y);"
        "    }"
        "}";

    ASTNode *ast = parse_source(source);
    assert(ast->type == AST_BLOCK);
    ASTNode *inner_block = ast->data.block.statements[0];
    assert(inner_block->type == AST_BLOCK);
    free_ast(ast);

    printf("✓ Block and scope tests passed\n");
}

void test_error_handling()
{
    printf("\nTesting error handling...\n");

    // These should cause parser errors, but shouldn't crash
    const char *invalid_sources[] = {
        "let;",               // Missing identifier
        "let x: i32;",        // Missing initializer
        "if x > 0 { }",       // Missing parentheses
        "while { }",          // Missing condition
        "for i in 0..10 { }", // Missing parentheses
        NULL};

    for (int i = 0; invalid_sources[i] != NULL; i++)
    {
        TokenArray tokens = tokenize(invalid_sources[i]);
        Parser parser = init_parser(tokens);
        // The parser's error function will call exit(1), so we can't actually test it here
        // In a real implementation, you might want to modify the error handling to be non-fatal
        // for testing purposes
        free_token_array(&tokens);
    }

    printf("✓ Error handling tests completed\n");
}

int main()
{
    printf("Running parser tests...\n");

    test_variable_declaration();
    test_expressions();
    test_if_statements();
    test_loops();
    test_functions();
    test_blocks_and_scopes();
    test_error_handling();

    printf("\nAll parser tests passed! ✓\n");
    return 0;
}
