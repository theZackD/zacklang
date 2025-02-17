#include "include/semantic.h"
#include "include/parser.h"
#include "include/ast.h"
#include "include/lexer.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <setjmp.h>

// Global jump buffer for error handling
static jmp_buf error_buf;
static int in_test = 0;

// Custom exit function for testing
void test_exit(int status)
{
    if (in_test)
    {
        longjmp(error_buf, status);
    }
    else
    {
        exit(status);
    }
}

// Helper function to parse source code and get AST
ASTNode *parse_source(const char *source)
{
    TokenArray tokens = tokenize(source);
    Parser parser = init_parser(tokens);
    ASTNode *ast = parse_program(&parser);
    free_token_array(&tokens);
    return ast;
}

// Helper function to check if semantic analysis causes an error
int check_semantic_error(const char *source)
{
    ASTNode *ast = parse_source(source);
    int had_error = 0;

    // Redirect stderr to capture error messages
    FILE *original_stderr = stderr;
    FILE *temp = tmpfile();
    stderr = temp;

    // Set up error handling
    in_test = 1;
    if (setjmp(error_buf) == 0)
    {
        semantic_analysis(ast);
    }
    else
    {
        had_error = 1;
    }
    in_test = 0;

    // Restore stderr
    stderr = original_stderr;
    fclose(temp);
    free_ast(ast);

    return had_error;
}

void test_variable_declarations()
{
    printf("\nTesting variable declarations...\n");

    // Valid declarations
    assert(check_semantic_error("let x: i32 = 42;") == 0);
    assert(check_semantic_error("let x: i32 = 42; let y: f64 = 3.14;") == 0);

    // Invalid: duplicate declaration in same scope
    assert(check_semantic_error("let x: i32 = 42; let x: f64 = 3.14;") == 1);

    // Valid: same name in different scopes (shadowing)
    assert(check_semantic_error("let x: i32 = 42; { let x: f64 = 3.14; }") == 0);

    // Invalid: missing type annotation
    assert(check_semantic_error("let x = 42;") == 1);

    printf("✓ Variable declaration tests passed\n");
}

void test_variable_usage()
{
    printf("\nTesting variable usage...\n");

    // Valid: use after declaration
    assert(check_semantic_error("let x: i32 = 42; let y: i32 = x;") == 0);

    // Invalid: use before declaration
    assert(check_semantic_error("let y: i32 = x; let x: i32 = 42;") == 1);

    // Invalid: undefined variable
    assert(check_semantic_error("let y: i32 = z;") == 1);

    printf("✓ Variable usage tests passed\n");
}

void test_function_declarations()
{
    printf("\nTesting function declarations...\n");

    // Valid function declaration
    assert(check_semantic_error("fn add(a: i32, b: i32): i32 { a + b }") == 0);

    // Invalid: duplicate function declaration
    assert(check_semantic_error(
               "fn add(a: i32, b: i32): i32 { a + b } "
               "fn add(x: f64, y: f64): f64 { x + y }") == 1);

    // Valid: function with multiple parameters
    assert(check_semantic_error(
               "fn calc(a: i32, b: i32, c: i32): i32 { a + b + c }") == 0);

    printf("✓ Function declaration tests passed\n");
}

void test_function_calls()
{
    printf("\nTesting function calls...\n");

    // Valid function call
    assert(check_semantic_error(
               "fn add(a: i32, b: i32): i32 { a + b } "
               "let x: i32 = add(1, 2);") == 0);

    // Invalid: undefined function
    assert(check_semantic_error("let x: i32 = add(1, 2);") == 1);

    // Invalid: wrong number of arguments
    assert(check_semantic_error(
               "fn add(a: i32, b: i32): i32 { a + b } "
               "let x: i32 = add(1);") == 1);

    printf("✓ Function call tests passed\n");
}

void test_scoping()
{
    printf("\nTesting scoping rules...\n");

    // Valid: nested scopes
    assert(check_semantic_error(
               "let x: i32 = 1; "
               "{ "
               "    let y: i32 = x; "
               "    { "
               "        let z: i32 = y; "
               "    } "
               "}") == 0);

    // Valid: shadowing in nested scope
    assert(check_semantic_error(
               "let x: i32 = 1; "
               "{ "
               "    let x: f64 = 2.0; "
               "}") == 0);

    // Invalid: use of variable outside its scope
    assert(check_semantic_error(
               "{ "
               "    let x: i32 = 1; "
               "} "
               "let y: i32 = x;") == 1);

    printf("✓ Scoping tests passed\n");
}

int main()
{
    printf("Running semantic analysis tests...\n");

    test_variable_declarations();
    test_variable_usage();
    test_function_declarations();
    test_function_calls();
    test_scoping();

    printf("\nAll semantic analysis tests passed! ✓\n");
    return 0;
}