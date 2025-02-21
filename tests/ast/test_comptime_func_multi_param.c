#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test a comptime function that takes multiple parameters
void test_function_with_multiple_parameters(void)
{
    // Create: comptime fn add(x: i32, y: i32): i32 { return x + y; }
    ASTNode *param_x = create_var_decl(1, "x", "i32", NULL); // Make it const, no initializer
    ASTNode *param_y = create_var_decl(1, "y", "i32", NULL); // Make it const, no initializer
    ASTNode **params = malloc(2 * sizeof(ASTNode *));
    params[0] = param_x;
    params[1] = param_y;

    // Create return x + y
    ASTNode *x_ref = create_identifier("x");
    ASTNode *y_ref = create_identifier("y");
    ASTNode *add = create_binary_expr("+", x_ref, y_ref);
    ASTNode *return_stmt = create_return_stmt(add);

    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = return_stmt;
    ASTNode *body = create_block(body_stmts, 1);
    ASTNode *func_def = create_func_def("add", params, 2, "i32", body, 1); // is_comptime = 1

    // Create symbol table and add function
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "add", "fn(i32, i32): i32", func_def);

    // Create and evaluate function call with arguments 5 and 3
    ASTNode *arg1 = create_literal("5");
    ASTNode *arg2 = create_literal("3");
    ASTNode **args = malloc(2 * sizeof(ASTNode *));
    args[0] = arg1;
    args[1] = arg2;
    ASTNode *call = create_func_call("add", args, 2);
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(call, table);

    // Verify result
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 8);
    printf("✓ Function with multiple parameters evaluation passed\n");

    // Cleanup
    free_comptime_value(result);
    free_ast(call);
    free_ast(func_def);
    destroy_symbol_table(table);
}

int main()
{
    printf("Running comptime function with multiple parameters tests...\n");
    test_function_with_multiple_parameters();
    printf("All function with multiple parameters tests passed!\n");
    return 0;
}