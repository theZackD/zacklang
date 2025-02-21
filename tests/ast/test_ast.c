#include "../../include/ast.h"
#include "../../include/semantic.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to print AST node type
void print_node_type(ASTNode *node)
{
    if (!node)
    {
        printf("NULL node\n");
        return;
    }

    const char *type_names[] = {
        "VAR_DECL", "PRINT_STMT", "PROMPT_STMT", "IF_STMT",
        "WHILE_STMT", "FOR_STMT", "FUNC_DEF", "EXPR_STMT",
        "BLOCK", "BINARY_EXPR", "UNARY_EXPR", "LITERAL",
        "IDENTIFIER", "FUNC_CALL", "ASSIGN_EXPR", "RETURN_STMT",
        "ARRAY_LITERAL", "ARRAY_INDEX"};

    printf("Node type: %s\n", type_names[node->type]);
}

// Test creation of basic AST nodes
void test_basic_nodes(void)
{
    // Test literal
    ASTNode *num = create_literal("42");
    assert(num->type == AST_LITERAL);
    assert(strcmp(num->data.literal.value, "42") == 0);

    // Test identifier
    ASTNode *id = create_identifier("x");
    assert(id->type == AST_IDENTIFIER);
    assert(strcmp(id->data.identifier.name, "x") == 0);

    // Test binary expression
    ASTNode *add = create_binary_expr("+", num, id);
    assert(add->type == AST_BINARY_EXPR);
    assert(strcmp(add->data.binary_expr.op, "+") == 0);

    free_ast(add); // This will free num and id as well
    printf("✓ Basic nodes test passed\n");
}

// Test array-related nodes
void test_array_nodes(void)
{
    // Create array literal [1, 2, 3]
    ASTNode **elements = malloc(3 * sizeof(ASTNode *));
    elements[0] = create_literal("1");
    elements[1] = create_literal("2");
    elements[2] = create_literal("3");

    ASTNode *array = create_array_literal(elements, 3);
    assert(array->type == AST_ARRAY_LITERAL);
    assert(array->data.array_literal.element_count == 3);

    // Test array indexing arr[1]
    ASTNode *index = create_literal("1");
    ASTNode *array_access = create_array_index(array, index);
    assert(array_access->type == AST_ARRAY_INDEX);

    free_ast(array_access); // This will free both array and index
    printf("✓ Array nodes test passed\n");
}

// Test comptime Fibonacci AST creation
void test_fibonacci_ast(void)
{
    // Create fibonacci(n: i32): i32 function
    ASTNode **params = malloc(sizeof(ASTNode *));
    params[0] = create_var_decl(0, "n", "i32", NULL);

    // Create if (n <= 1) condition
    ASTNode *n_id = create_identifier("n");
    ASTNode *one = create_literal("1");
    ASTNode *condition = create_binary_expr("<=", n_id, one);

    // Create return n for base case
    ASTNode *return_n = create_return_stmt(create_identifier("n"));
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = return_n;
    ASTNode *if_block = create_block(if_stmts, 1);

    // Create recursive case: return fibonacci(n-1) + fibonacci(n-2)
    ASTNode *n_minus_1 = create_binary_expr("-", create_identifier("n"), create_literal("1"));
    ASTNode *n_minus_2 = create_binary_expr("-", create_identifier("n"), create_literal("2"));

    ASTNode **fib1_args = malloc(sizeof(ASTNode *));
    fib1_args[0] = n_minus_1;
    ASTNode *fib1_call = create_func_call("fibonacci", fib1_args, 1);

    ASTNode **fib2_args = malloc(sizeof(ASTNode *));
    fib2_args[0] = n_minus_2;
    ASTNode *fib2_call = create_func_call("fibonacci", fib2_args, 1);

    ASTNode *sum = create_binary_expr("+", fib1_call, fib2_call);
    ASTNode *return_sum = create_return_stmt(sum);

    // Create function body
    ASTNode **body_stmts = malloc(2 * sizeof(ASTNode *));
    body_stmts[0] = create_if_stmt(condition, if_block, NULL, NULL, 0, NULL);
    body_stmts[1] = return_sum;
    ASTNode *body = create_block(body_stmts, 2);

    // Create the complete function
    ASTNode *fibonacci = create_func_def("fibonacci", params, 1, "i32", body, 1); // is_comptime = 1

    // Verify the structure
    assert(fibonacci->type == AST_FUNC_DEF);
    assert(fibonacci->data.func_def.is_comptime == 1);
    assert(strcmp(fibonacci->data.func_def.name, "fibonacci") == 0);
    assert(fibonacci->data.func_def.param_count == 1);
    assert(strcmp(fibonacci->data.func_def.return_type, "i32") == 0);

    // Run semantic analysis
    semantic_analysis(fibonacci);

    free_ast(fibonacci);
    printf("✓ Fibonacci AST test passed\n");
}

// Test main function that uses fibonacci
void test_main_function_ast(void)
{
    // Create print(f"Fibonacci(10) = {fibonacci(10)}")
    ASTNode **fib_args = malloc(sizeof(ASTNode *));
    fib_args[0] = create_literal("10");
    ASTNode *fib_call = create_func_call("fibonacci", fib_args, 1);

    ASTNode **print_args = malloc(sizeof(ASTNode *));
    print_args[0] = create_literal("f\"Fibonacci(10) = {fibonacci(10)}\"");
    ASTNode *print_stmt = create_print_stmt(print_args[0]);

    // Create main function
    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = print_stmt;
    ASTNode *body = create_block(body_stmts, 1);

    ASTNode *main_func = create_func_def("main", NULL, 0, "void", body, 0);

    // Verify structure
    assert(main_func->type == AST_FUNC_DEF);
    assert(strcmp(main_func->data.func_def.name, "main") == 0);
    assert(main_func->data.func_def.param_count == 0);
    assert(strcmp(main_func->data.func_def.return_type, "void") == 0);

    // Run semantic analysis
    semantic_analysis(main_func);

    free_ast(main_func);
    printf("✓ Main function AST test passed\n");
}

int main()
{
    printf("Running AST tests...\n");

    test_basic_nodes();
    test_array_nodes();
    test_fibonacci_ast();
    test_main_function_ast();

    printf("All AST tests passed!\n");
    return 0;
}