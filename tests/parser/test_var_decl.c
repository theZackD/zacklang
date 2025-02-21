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
    ASTNode *node = parse_var_declaration(parser);
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

// Test basic variable declaration
void test_basic_declaration(void)
{
    // Test simple integer declaration
    ASTNode *node = parse_string("let x = 42;");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(strcmp(node->data.var_decl.identifier, "x") == 0);
    assert(node->data.var_decl.is_const == 0);
    assert(node->data.var_decl.type_annotation == NULL);
    assert(node->data.var_decl.initializer->type == AST_LITERAL);
    assert(strcmp(node->data.var_decl.initializer->data.literal.value, "42") == 0);
    free_ast(node);
    printf("✓ Basic integer declaration test passed\n");

    // Test float declaration
    node = parse_string("let y = 3.14;");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(strcmp(node->data.var_decl.identifier, "y") == 0);
    assert(node->data.var_decl.initializer->type == AST_LITERAL);
    assert(strcmp(node->data.var_decl.initializer->data.literal.value, "3.14") == 0);
    free_ast(node);
    printf("✓ Basic float declaration test passed\n");

    // Test string declaration
    node = parse_string("let msg = \"hello\";");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(strcmp(node->data.var_decl.identifier, "msg") == 0);
    assert(node->data.var_decl.initializer->type == AST_LITERAL);
    assert(strcmp(node->data.var_decl.initializer->data.literal.value, "hello") == 0);
    free_ast(node);
    printf("✓ Basic string declaration test passed\n");
}

// Test const declarations
void test_const_declaration(void)
{
    // Test const integer
    ASTNode *node = parse_string("let const x = 42;");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(node->data.var_decl.is_const == 1);
    assert(strcmp(node->data.var_decl.identifier, "x") == 0);
    free_ast(node);
    printf("✓ Const integer declaration test passed\n");

    // Test const with type annotation
    node = parse_string("let const pi: f64 = 3.14159;");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(node->data.var_decl.is_const == 1);
    assert(strcmp(node->data.var_decl.identifier, "pi") == 0);
    assert(strcmp(node->data.var_decl.type_annotation, "f64") == 0);
    free_ast(node);
    printf("✓ Const with type annotation test passed\n");
}

// Test type annotations
void test_type_annotations(void)
{
    // Test integer type
    ASTNode *node = parse_string("let count: i32 = 0;");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(strcmp(node->data.var_decl.type_annotation, "i32") == 0);
    free_ast(node);
    printf("✓ Integer type annotation test passed\n");

    // Test string type
    node = parse_string("let name: string = \"John\";");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(strcmp(node->data.var_decl.type_annotation, "string") == 0);
    free_ast(node);
    printf("✓ String type annotation test passed\n");

    // Test boolean type
    node = parse_string("let flag: bool = true;");
    assert(node != NULL);
    assert(node->type == AST_VAR_DECL);
    assert(strcmp(node->data.var_decl.type_annotation, "bool") == 0);
    free_ast(node);
    printf("✓ Boolean type annotation test passed\n");
}

// Test error cases
void test_error_cases(void)
{
    // Missing let keyword
    assert(parse_string("x = 42;") == NULL);
    printf("✓ Missing 'let' keyword test passed\n");

    // Missing identifier
    assert(parse_string("let = 42;") == NULL);
    printf("✓ Missing identifier test passed\n");

    // Missing equals sign
    assert(parse_string("let x 42;") == NULL);
    printf("✓ Missing equals sign test passed\n");

    // Missing semicolon
    assert(parse_string("let x = 42") == NULL);
    printf("✓ Missing semicolon test passed\n");

    // Invalid type annotation
    assert(parse_string("let x: invalid = 42;") == NULL);
    printf("✓ Invalid type annotation test passed\n");

    // Missing expression
    assert(parse_string("let x = ;") == NULL);
    printf("✓ Missing expression test passed\n");

    // Missing type after colon
    assert(parse_string("let x: = 42;") == NULL);
    printf("✓ Missing type after colon test passed\n");
}

int main()
{
    printf("Running variable declaration parser tests...\n");

    test_basic_declaration();
    test_const_declaration();
    test_type_annotations();
    test_error_cases();

    printf("All variable declaration tests passed!\n");
    return 0;
}