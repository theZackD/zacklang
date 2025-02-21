#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test recursive comptime function calls
void test_recursive_factorial(void)
{
    // Create factorial function:
    // comptime fn factorial(n: i32): i32 {
    //     if (n <= 1) {
    //         return 1;
    //     }
    //     return n * factorial(n - 1);
    // }

    // Create parameter
    ASTNode *param = create_var_decl(1, "n", "i32", NULL);
    ASTNode **params = malloc(sizeof(ASTNode *));
    params[0] = param;

    // Create if condition: n <= 1
    ASTNode *n_ref = create_identifier("n");
    ASTNode *one = create_literal("1");
    ASTNode *condition = create_binary_expr("<=", n_ref, one);

    // Create if block: return 1
    ASTNode *return_one = create_return_stmt(create_literal("1"));
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = return_one;
    ASTNode *if_block = create_block(if_stmts, 1);

    // Create recursive case: return n * factorial(n - 1)
    // First create factorial(n - 1)
    ASTNode *n_ref2 = create_identifier("n");
    ASTNode *one2 = create_literal("1");
    ASTNode *sub = create_binary_expr("-", n_ref2, one2);
    ASTNode **rec_args = malloc(sizeof(ASTNode *));
    rec_args[0] = sub;
    ASTNode *rec_call = create_func_call("factorial", rec_args, 1);

    // Create n * factorial(n - 1)
    ASTNode *n_ref3 = create_identifier("n");
    ASTNode *mul = create_binary_expr("*", n_ref3, rec_call);
    ASTNode *return_rec = create_return_stmt(mul);

    // Create function body with if statement and recursive return
    ASTNode **body_stmts = malloc(2 * sizeof(ASTNode *));
    body_stmts[0] = create_if_stmt(condition, if_block, NULL, NULL, 0, NULL);
    body_stmts[1] = return_rec;
    ASTNode *body = create_block(body_stmts, 2);

    // Create the factorial function
    ASTNode *factorial = create_func_def("factorial", params, 1, "i32", body, 1);

    // Create symbol table and add function
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "factorial", "fn(i32): i32", factorial);

    // Test factorial(5) which should return 120
    ASTNode *test_arg = create_literal("5");
    ASTNode **test_args = malloc(sizeof(ASTNode *));
    test_args[0] = test_arg;
    ASTNode *test_call = create_func_call("factorial", test_args, 1);
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(test_call, table);

    // Verify result
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 120);
    printf("✓ Recursive factorial evaluation passed\n");

    // Cleanup
    free_comptime_value(result);
    free_ast(test_call);
    free_ast(factorial);
    destroy_symbol_table(table);
}

int main()
{
    printf("Running recursive comptime function tests...\n");
    test_recursive_factorial();
    printf("All recursive function tests passed!\n");
    return 0;
}