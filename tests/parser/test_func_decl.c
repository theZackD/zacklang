#include "../../include/parser.h"
#include "../../include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function to parse a string and return the AST
static ASTNode *parse_string(const char *input)
{
    TokenArray tokens = tokenize(input);
    Parser *parser = create_parser(tokens);
    ASTNode *node = parse_function_declaration(parser);
    int had_error = parser->had_error;
    destroy_parser(parser);
    free_token_array(&tokens);
    if (had_error)
    {
        if (node)
            free_ast(node);
        return NULL;
    }
    return node;
}

// Test basic function declarations
void test_basic_function(void)
{
    // Test empty function
    ASTNode *node = parse_string("fn empty() { let x = 42; }");
    assert(node != NULL);
    assert(node->type == AST_FUNC_DEF);
    assert(strcmp(node->data.func_def.name, "empty") == 0);
    assert(node->data.func_def.param_count == 0);
    assert(node->data.func_def.return_type == NULL);
    assert(node->data.func_def.is_comptime == 0);
    assert(node->data.func_def.body != NULL);
    assert(node->data.func_def.body->type == AST_BLOCK);
    assert(node->data.func_def.body->data.block.stmt_count == 1);
    free_ast(node);
    printf("✓ Basic empty function test passed\n");

    // Test function with return type
    node = parse_string("fn add(): i32 { let x = 42; }");
    assert(node != NULL);
    assert(node->type == AST_FUNC_DEF);
    assert(strcmp(node->data.func_def.name, "add") == 0);
    assert(strcmp(node->data.func_def.return_type, "i32") == 0);
    free_ast(node);
    printf("✓ Function with return type test passed\n");
}

// Test function parameters
void test_function_parameters(void)
{
    // Test single parameter
    ASTNode *node = parse_string("fn inc(x: i32): i32 { let y = 42; }");
    assert(node != NULL);
    assert(node->type == AST_FUNC_DEF);
    assert(node->data.func_def.param_count == 1);
    assert(node->data.func_def.parameters[0]->type == AST_VAR_DECL);
    assert(strcmp(node->data.func_def.parameters[0]->data.var_decl.identifier, "x") == 0);
    assert(strcmp(node->data.func_def.parameters[0]->data.var_decl.type_annotation, "i32") == 0);
    free_ast(node);
    printf("✓ Single parameter test passed\n");

    // Test multiple parameters
    node = parse_string("fn add(x: i32, y: i32): i32 { let z = 42; }");
    assert(node != NULL);
    assert(node->type == AST_FUNC_DEF);
    assert(node->data.func_def.param_count == 2);
    assert(strcmp(node->data.func_def.parameters[0]->data.var_decl.identifier, "x") == 0);
    assert(strcmp(node->data.func_def.parameters[1]->data.var_decl.identifier, "y") == 0);
    free_ast(node);
    printf("✓ Multiple parameters test passed\n");

    // Test parameters without type annotations
    node = parse_string("fn test(x, y) { let z = 42; }");
    assert(node != NULL);
    assert(node->type == AST_FUNC_DEF);
    assert(node->data.func_def.param_count == 2);
    assert(node->data.func_def.parameters[0]->data.var_decl.type_annotation == NULL);
    assert(node->data.func_def.parameters[1]->data.var_decl.type_annotation == NULL);
    free_ast(node);
    printf("✓ Parameters without type annotations test passed\n");
}

// Test comptime functions
void test_comptime_function(void)
{
    // Test basic comptime function
    ASTNode *node = parse_string("comptime fn factorial(n: i32): i32 { let x = 42; }");
    assert(node != NULL);
    assert(node->type == AST_FUNC_DEF);
    assert(node->data.func_def.is_comptime == 1);
    assert(strcmp(node->data.func_def.name, "factorial") == 0);
    assert(node->data.func_def.param_count == 1);
    assert(strcmp(node->data.func_def.return_type, "i32") == 0);
    free_ast(node);
    printf("✓ Comptime function test passed\n");
}

// Test error cases
void test_error_cases(void)
{
    // Missing function name
    assert(parse_string("fn () { }") == NULL);
    printf("✓ Missing function name test passed\n");

    // Missing parameter name
    assert(parse_string("fn test(:i32) { }") == NULL);
    printf("✓ Missing parameter name test passed\n");

    // Missing parameter type after colon
    assert(parse_string("fn test(x:) { }") == NULL);
    printf("✓ Missing parameter type test passed\n");

    // Missing closing parenthesis
    assert(parse_string("fn test(x: i32 { }") == NULL);
    printf("✓ Missing closing parenthesis test passed\n");

    // Missing opening brace
    assert(parse_string("fn test() }") == NULL);
    printf("✓ Missing opening brace test passed\n");

    // Missing closing brace
    assert(parse_string("fn test() {") == NULL);
    printf("✓ Missing closing brace test passed\n");

    // Invalid return type
    assert(parse_string("fn test(): invalid { }") == NULL);
    printf("✓ Invalid return type test passed\n");

    // Missing return type after colon
    assert(parse_string("fn test(): { }") == NULL);
    printf("✓ Missing return type after colon test passed\n");
}

int main()
{
    printf("Running function declaration parser tests...\n");

    test_basic_function();
    test_function_parameters();
    test_comptime_function();
    test_error_cases();

    printf("All function declaration tests passed!\n");
    return 0;
}