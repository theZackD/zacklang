#include "../../include/parser.h"
#include "../../include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function to print tokens
static void debug_print_tokens(const TokenArray *tokens)
{
    for (int i = 0; i < tokens->count; i++)
    {
        printf("DEBUG:   Token[%d]: type=%d, value='%s'\n",
               i, tokens->tokens[i].type, tokens->tokens[i].value);
    }
}

// Helper function to parse a string and return the AST
static ASTNode *parse_string(const char *input)
{
    printf("\nDEBUG: Parsing input: '%s'\n", input);
    TokenArray tokens = tokenize(input);
    printf("DEBUG: Tokens:\n");
    debug_print_tokens(&tokens);

    Parser *parser = create_parser(tokens);
    ASTNode *node = parse_expression(parser);
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

// Test primary expressions
void test_primary_expressions(void)
{
    // Test integer literal
    ASTNode *node = parse_string("42");
    assert(node != NULL);
    assert(node->type == AST_LITERAL);
    assert(strcmp(node->data.literal.value, "42") == 0);
    free_ast(node);
    printf("✓ Integer literal test passed\n");

    // Test float literal
    node = parse_string("3.14");
    assert(node != NULL);
    assert(node->type == AST_LITERAL);
    assert(strcmp(node->data.literal.value, "3.14") == 0);
    free_ast(node);
    printf("✓ Float literal test passed\n");

    // Test string literal
    node = parse_string("\"hello\"");
    assert(node != NULL);
    assert(node->type == AST_LITERAL);
    assert(strcmp(node->data.literal.value, "hello") == 0);
    free_ast(node);
    printf("✓ String literal test passed\n");

    // Test boolean literals
    node = parse_string("true");
    assert(node != NULL);
    assert(node->type == AST_LITERAL);
    assert(strcmp(node->data.literal.value, "true") == 0);
    free_ast(node);
    printf("✓ Boolean true literal test passed\n");

    node = parse_string("false");
    assert(node != NULL);
    assert(node->type == AST_LITERAL);
    assert(strcmp(node->data.literal.value, "false") == 0);
    free_ast(node);
    printf("✓ Boolean false literal test passed\n");

    // Test identifier
    node = parse_string("variable");
    assert(node != NULL);
    assert(node->type == AST_IDENTIFIER);
    assert(strcmp(node->data.identifier.name, "variable") == 0);
    free_ast(node);
    printf("✓ Identifier test passed\n");

    // Test parenthesized expression
    node = parse_string("(42)");
    assert(node != NULL);
    assert(node->type == AST_LITERAL);
    assert(strcmp(node->data.literal.value, "42") == 0);
    free_ast(node);
    printf("✓ Parenthesized expression test passed\n");
}

// Test unary operators
void test_unary_operators(void)
{
    // Test unary minus
    ASTNode *node = parse_string("-42");
    assert(node != NULL);
    assert(node->type == AST_UNARY_EXPR);
    assert(strcmp(node->data.unary_expr.op, "-") == 0);
    assert(node->data.unary_expr.operand->type == AST_LITERAL);
    assert(strcmp(node->data.unary_expr.operand->data.literal.value, "42") == 0);
    free_ast(node);
    printf("✓ Unary minus test passed\n");

    // Test unary plus
    node = parse_string("+42");
    assert(node != NULL);
    assert(node->type == AST_UNARY_EXPR);
    assert(strcmp(node->data.unary_expr.op, "+") == 0);
    free_ast(node);
    printf("✓ Unary plus test passed\n");

    // Test logical not
    node = parse_string("not true");
    assert(node != NULL);
    assert(node->type == AST_UNARY_EXPR);
    assert(strcmp(node->data.unary_expr.op, "not") == 0);
    free_ast(node);
    printf("✓ Logical not test passed\n");

    // Test nested unary operators
    node = parse_string("not -42");
    assert(node != NULL);
    assert(node->type == AST_UNARY_EXPR);
    assert(strcmp(node->data.unary_expr.op, "not") == 0);
    assert(node->data.unary_expr.operand->type == AST_UNARY_EXPR);
    assert(strcmp(node->data.unary_expr.operand->data.unary_expr.op, "-") == 0);
    free_ast(node);
    printf("✓ Nested unary operators test passed\n");
}

// Test binary operators and precedence
void test_binary_operators(void)
{
    // Test addition
    ASTNode *node = parse_string("1 + 2");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "+") == 0);
    free_ast(node);
    printf("✓ Addition test passed\n");

    // Test multiplication precedence
    node = parse_string("1 + 2 * 3");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "+") == 0);
    assert(node->data.binary_expr.right->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.right->data.binary_expr.op, "*") == 0);
    free_ast(node);
    printf("✓ Multiplication precedence test passed\n");

    // Test power operator (right associative)
    node = parse_string("2 ** 3 ** 2");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "**") == 0);
    assert(node->data.binary_expr.right->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.right->data.binary_expr.op, "**") == 0);
    free_ast(node);
    printf("✓ Power operator test passed\n");

    // Test comparison operators
    node = parse_string("1 < 2 and 3 > 4");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "and") == 0);
    assert(node->data.binary_expr.left->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.left->data.binary_expr.op, "<") == 0);
    assert(node->data.binary_expr.right->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.right->data.binary_expr.op, ">") == 0);
    free_ast(node);
    printf("✓ Comparison operators test passed\n");

    // Test equality operators
    node = parse_string("x == y and z != w");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "and") == 0);
    assert(node->data.binary_expr.left->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.left->data.binary_expr.op, "==") == 0);
    assert(node->data.binary_expr.right->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.right->data.binary_expr.op, "!=") == 0);
    free_ast(node);
    printf("✓ Equality operators test passed\n");
}

// Test logical operators
void test_logical_operators(void)
{
    // Test AND
    ASTNode *node = parse_string("true and false");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "and") == 0);
    free_ast(node);
    printf("✓ Logical AND test passed\n");

    // Test OR
    node = parse_string("true or false");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "or") == 0);
    free_ast(node);
    printf("✓ Logical OR test passed\n");

    // Test precedence (AND binds tighter than OR)
    node = parse_string("a or b and c");
    assert(node != NULL);
    assert(node->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.op, "or") == 0);
    assert(node->data.binary_expr.right->type == AST_BINARY_EXPR);
    assert(strcmp(node->data.binary_expr.right->data.binary_expr.op, "and") == 0);
    free_ast(node);
    printf("✓ Logical operator precedence test passed\n");
}

// Test assignment expressions
void test_assignment(void)
{
    // Test simple assignment
    ASTNode *node = parse_string("x = 42");
    assert(node != NULL);
    assert(node->type == AST_ASSIGN_EXPR);
    assert(node->data.assign_expr.left->type == AST_IDENTIFIER);
    assert(node->data.assign_expr.right->type == AST_LITERAL);
    free_ast(node);
    printf("✓ Simple assignment test passed\n");

    // Test chained assignment
    node = parse_string("x = y = 42");
    assert(node != NULL);
    assert(node->type == AST_ASSIGN_EXPR);
    assert(node->data.assign_expr.right->type == AST_ASSIGN_EXPR);
    free_ast(node);
    printf("✓ Chained assignment test passed\n");

    // Test invalid assignment target
    assert(parse_string("42 = x") == NULL);
    printf("✓ Invalid assignment target test passed\n");
}

// Test error cases
void test_error_cases(void)
{
    // Test unclosed parenthesis
    assert(parse_string("(42") == NULL);
    printf("✓ Unclosed parenthesis test passed\n");

    // Test invalid operator sequence
    assert(parse_string("42 + * 10") == NULL);
    printf("✓ Invalid operator sequence test passed\n");

    // Test invalid unary operator
    assert(parse_string("* 42") == NULL);
    printf("✓ Invalid unary operator test passed\n");

    // Test missing operand
    assert(parse_string("42 +") == NULL);
    printf("✓ Missing operand test passed\n");
}

int main()
{
    printf("Running expression parser tests...\n");

    test_primary_expressions();
    test_unary_operators();
    test_binary_operators();
    test_logical_operators();
    test_assignment();
    test_error_cases();

    printf("All expression tests passed!\n");
    return 0;
}