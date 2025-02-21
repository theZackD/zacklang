#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to create the fibonacci function AST
static ASTNode *create_fibonacci_function(void)
{
    // Create: comptime fn fibonacci(n: i32): i32 {
    //     if (n <= 1) {
    //         return n;
    //     }
    //     return fibonacci(n - 1) + fibonacci(n - 2);
    // }

    // Create parameter
    ASTNode *param = create_var_decl(1, "n", "i32", NULL);
    ASTNode **params = malloc(sizeof(ASTNode *));
    params[0] = param;

    // Create if condition: n <= 1
    ASTNode *n_ref = create_identifier("n");
    ASTNode *one = create_literal("1");
    ASTNode *condition = create_binary_expr("<=", n_ref, one);

    // Create if block: return n
    ASTNode *return_n = create_return_stmt(create_identifier("n"));
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = return_n;
    ASTNode *if_block = create_block(if_stmts, 1);

    // Create recursive case: return fibonacci(n - 1) + fibonacci(n - 2)
    // First create fibonacci(n - 1)
    ASTNode *n_ref2 = create_identifier("n");
    ASTNode *one2 = create_literal("1");
    ASTNode *sub1 = create_binary_expr("-", n_ref2, one2);
    ASTNode **rec_args1 = malloc(sizeof(ASTNode *));
    rec_args1[0] = sub1;
    ASTNode *rec_call1 = create_func_call("fibonacci", rec_args1, 1);

    // Then create fibonacci(n - 2)
    ASTNode *n_ref3 = create_identifier("n");
    ASTNode *two = create_literal("2");
    ASTNode *sub2 = create_binary_expr("-", n_ref3, two);
    ASTNode **rec_args2 = malloc(sizeof(ASTNode *));
    rec_args2[0] = sub2;
    ASTNode *rec_call2 = create_func_call("fibonacci", rec_args2, 1);

    // Add the recursive calls
    ASTNode *add = create_binary_expr("+", rec_call1, rec_call2);
    ASTNode *return_rec = create_return_stmt(add);

    // Create function body with if statement and recursive return
    ASTNode **body_stmts = malloc(2 * sizeof(ASTNode *));
    body_stmts[0] = create_if_stmt(condition, if_block, NULL, NULL, 0, NULL);
    body_stmts[1] = return_rec;
    ASTNode *body = create_block(body_stmts, 2);

    // Create the fibonacci function
    return create_func_def("fibonacci", params, 1, "i32", body, 1);
}

// Helper function to test fibonacci with a specific input
static void test_fibonacci_n(SymbolTable *table, int n, int expected)
{
    char n_str[32];
    snprintf(n_str, sizeof(n_str), "%d", n);

    ASTNode *test_arg = create_literal(n_str);
    ASTNode **test_args = malloc(sizeof(ASTNode *));
    test_args[0] = test_arg;
    ASTNode *test_call = create_func_call("fibonacci", test_args, 1);

    printf("Testing fibonacci(%d)...\n", n);
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(test_call, table);

    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == expected);
    printf("✓ fibonacci(%d) = %d passed\n", n, expected);

    free_comptime_value(result);
    free_ast(test_call);
}

// Test fibonacci with various inputs
void test_fibonacci(void)
{
    // Create fibonacci function
    ASTNode *fibonacci = create_fibonacci_function();

    // Create symbol table and add function
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "fibonacci", "fn(i32): i32", fibonacci);

    // Test base cases
    test_fibonacci_n(table, 0, 0); // fib(0) = 0
    test_fibonacci_n(table, 1, 1); // fib(1) = 1

    // Test small numbers
    test_fibonacci_n(table, 2, 1); // fib(2) = 1
    test_fibonacci_n(table, 3, 2); // fib(3) = 2
    test_fibonacci_n(table, 4, 3); // fib(4) = 3
    test_fibonacci_n(table, 5, 5); // fib(5) = 5

    // Test medium numbers
    test_fibonacci_n(table, 6, 8);  // fib(6) = 8
    test_fibonacci_n(table, 7, 13); // fib(7) = 13
    test_fibonacci_n(table, 8, 21); // fib(8) = 21

    // Test larger numbers
    test_fibonacci_n(table, 9, 34);   // fib(9) = 34
    test_fibonacci_n(table, 10, 55);  // fib(10) = 55
    test_fibonacci_n(table, 11, 89);  // fib(11) = 89
    test_fibonacci_n(table, 12, 144); // fib(12) = 144

    // Cleanup
    free_ast(fibonacci);
    destroy_symbol_table(table);
}

int main()
{
    printf("Running fibonacci tests...\n");
    test_fibonacci();
    printf("All fibonacci tests passed!\n");
    return 0;
}