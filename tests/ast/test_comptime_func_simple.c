#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test a simple comptime function that returns a literal
void test_simple_function(void)
{
    // Create: comptime fn answer(): i32 { return 42; }
    ASTNode *return_stmt = create_return_stmt(create_literal("42"));
    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = return_stmt;
    ASTNode *body = create_block(body_stmts, 1);
    ASTNode *func_def = create_func_def("answer", NULL, 0, "i32", body, 1); // is_comptime = 1

    // Create symbol table and add function
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "answer", "fn(): i32", func_def);

    // Create and evaluate function call
    ASTNode *call = create_func_call("answer", NULL, 0);
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(call, table);

    // Verify result
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 42);
    printf("✓ Simple function evaluation passed\n");

    // Cleanup
    free_comptime_value(result);
    free_ast(call);
    free_ast(func_def);
    destroy_symbol_table(table);
}

int main()
{
    printf("Running simple comptime function tests...\n");
    test_simple_function();
    printf("All simple function tests passed!\n");
    return 0;
}