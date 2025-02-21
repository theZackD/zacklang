#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test a comptime function that takes a single parameter
void test_function_with_parameter(void)
{
    // Create: comptime fn inc(x: i32): i32 { return x + 1; }
    ASTNode *param_x = create_var_decl(1, "x", "i32", NULL); // Make it const, no initializer
    ASTNode **params = malloc(sizeof(ASTNode *));
    params[0] = param_x;

    // Create return x + 1
    ASTNode *x_ref = create_identifier("x");
    ASTNode *one = create_literal("1");
    ASTNode *add = create_binary_expr("+", x_ref, one);
    ASTNode *return_stmt = create_return_stmt(add);

    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = return_stmt;
    ASTNode *body = create_block(body_stmts, 1);
    ASTNode *func_def = create_func_def("inc", params, 1, "i32", body, 1); // is_comptime = 1

    // Create symbol table and add function
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "inc", "fn(i32): i32", func_def);

    // Create and evaluate function call with argument 5
    ASTNode *arg = create_literal("5");
    ASTNode **args = malloc(sizeof(ASTNode *));
    args[0] = arg;
    ASTNode *call = create_func_call("inc", args, 1);
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(call, table);

    // Verify result
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 6);
    printf("✓ Function with parameter evaluation passed\n");

    // Cleanup
    free_comptime_value(result);
    free_ast(call);
    free_ast(func_def);
    destroy_symbol_table(table);
}

int main()
{
    printf("Running comptime function with parameter tests...\n");
    test_function_with_parameter();
    printf("All function with parameter tests passed!\n");
    return 0;
}