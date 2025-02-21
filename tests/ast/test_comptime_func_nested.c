#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test nested comptime function calls
void test_nested_function_calls(void)
{
    // Create two functions:
    // comptime fn double(x: i32): i32 { return x + x; }
    // comptime fn quadruple(x: i32): i32 { return double(double(x)); }

    // First, create the double function
    ASTNode *double_param = create_var_decl(1, "x", "i32", NULL);
    ASTNode **double_params = malloc(sizeof(ASTNode *));
    double_params[0] = double_param;

    // Create return x + x
    ASTNode *x_ref1 = create_identifier("x");
    ASTNode *x_ref2 = create_identifier("x");
    ASTNode *add = create_binary_expr("+", x_ref1, x_ref2);
    ASTNode *double_return = create_return_stmt(add);

    ASTNode **double_body_stmts = malloc(sizeof(ASTNode *));
    double_body_stmts[0] = double_return;
    ASTNode *double_body = create_block(double_body_stmts, 1);
    ASTNode *double_func = create_func_def("double", double_params, 1, "i32", double_body, 1);

    // Now create the quadruple function
    ASTNode *quad_param = create_var_decl(1, "x", "i32", NULL);
    ASTNode **quad_params = malloc(sizeof(ASTNode *));
    quad_params[0] = quad_param;

    // Create inner double call: double(x)
    ASTNode *inner_arg = create_identifier("x");
    ASTNode **inner_args = malloc(sizeof(ASTNode *));
    inner_args[0] = inner_arg;
    ASTNode *inner_call = create_func_call("double", inner_args, 1);

    // Create outer double call: double(double(x))
    ASTNode **outer_args = malloc(sizeof(ASTNode *));
    outer_args[0] = inner_call;
    ASTNode *outer_call = create_func_call("double", outer_args, 1);

    // Create return statement
    ASTNode *quad_return = create_return_stmt(outer_call);
    ASTNode **quad_body_stmts = malloc(sizeof(ASTNode *));
    quad_body_stmts[0] = quad_return;
    ASTNode *quad_body = create_block(quad_body_stmts, 1);
    ASTNode *quad_func = create_func_def("quadruple", quad_params, 1, "i32", quad_body, 1);

    // Create symbol table and add both functions
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "double", "fn(i32): i32", double_func);
    add_symbol_with_node(table, "quadruple", "fn(i32): i32", quad_func);

    // Test quadruple(3) which should return 12
    ASTNode *test_arg = create_literal("3");
    ASTNode **test_args = malloc(sizeof(ASTNode *));
    test_args[0] = test_arg;
    ASTNode *test_call = create_func_call("quadruple", test_args, 1);
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(test_call, table);

    // Verify result
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 12);
    printf("✓ Nested function calls evaluation passed\n");

    // Cleanup
    free_comptime_value(result);
    free_ast(test_call);
    free_ast(double_func);
    free_ast(quad_func);
    destroy_symbol_table(table);
}

int main()
{
    printf("Running nested comptime function calls tests...\n");
    test_nested_function_calls();
    printf("All nested function calls tests passed!\n");
    return 0;
}