#include "include/ast_to_zir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

// Helper function to create a simple AST for testing
static ASTNode *create_test_ast_function(void)
{
    // Create a function "add" that takes two i32 parameters and returns i32
    ASTNode **params = malloc(2 * sizeof(ASTNode *));
    params[0] = create_var_decl(0, "a", "i32", NULL);
    params[1] = create_var_decl(0, "b", "i32", NULL);

    // Create function body: return a + b;
    ASTNode *a_id = create_identifier("a");
    ASTNode *b_id = create_identifier("b");
    ASTNode *add_expr = create_binary_expr("+", a_id, b_id);
    ASTNode *return_stmt = create_return_stmt(add_expr);

    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = return_stmt;
    ASTNode *body = create_block(body_stmts, 1);

    // Create the function definition
    ASTNode *func = create_func_def("add", params, 2, "i32", body);

    // Create a block containing just this function
    ASTNode **stmts = malloc(sizeof(ASTNode *));
    stmts[0] = func;
    return create_block(stmts, 1);
}

// Helper function to create a test AST with variables and control flow
static ASTNode *create_test_ast_control_flow(void)
{
    // Create a function that demonstrates control flow
    // fn max(a: i32, b: i32) -> i32 {
    //     if (a > b) {
    //         return a;
    //     } else {
    //         return b;
    //     }
    // }
    ASTNode **params = malloc(2 * sizeof(ASTNode *));
    params[0] = create_var_decl(0, "a", "i32", NULL);
    params[1] = create_var_decl(0, "b", "i32", NULL);

    // Create condition: a > b
    ASTNode *a_id = create_identifier("a");
    ASTNode *b_id = create_identifier("b");
    ASTNode *cond = create_binary_expr(">", a_id, b_id);

    // Create then block: return a
    ASTNode *then_return = create_return_stmt(create_identifier("a"));
    ASTNode **then_stmts = malloc(sizeof(ASTNode *));
    then_stmts[0] = then_return;
    ASTNode *then_block = create_block(then_stmts, 1);

    // Create else block: return b
    ASTNode *else_return = create_return_stmt(create_identifier("b"));
    ASTNode **else_stmts = malloc(sizeof(ASTNode *));
    else_stmts[0] = else_return;
    ASTNode *else_block = create_block(else_stmts, 1);

    // Create if statement
    ASTNode *if_stmt = create_if_stmt(cond, then_block, else_block);

    // Create function body
    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = if_stmt;
    ASTNode *body = create_block(body_stmts, 1);

    // Create the function definition
    ASTNode *func = create_func_def("max", params, 2, "i32", body);

    // Create a block containing just this function
    ASTNode **stmts = malloc(sizeof(ASTNode *));
    stmts[0] = func;
    return create_block(stmts, 1);
}

void test_simple_function_translation(void)
{
    printf("\nTesting simple function translation...\n");

    // Create test AST
    ASTNode *ast = create_test_ast_function();

    // Translate to ZIR
    ZIRModule *module = translate_ast_to_zir(ast);
    assert(module != NULL);
    assert(module->func_count == 2); // main + add function

    // Verify function
    ZIRFunction *func = NULL;
    for (int i = 0; i < module->func_count; i++)
    {
        if (strcmp(module->functions[i]->name, "add") == 0)
        {
            func = module->functions[i];
            break;
        }
    }
    assert(func != NULL);
    assert(func->param_count == 2);
    assert(func->return_type->kind == TYPE_I32);

    // Verify parameters
    assert(strcmp(func->params[0].name, "a") == 0);
    assert(func->params[0].type->kind == TYPE_I32);
    assert(strcmp(func->params[1].name, "b") == 0);
    assert(func->params[1].type->kind == TYPE_I32);

    // Verify entry block exists
    assert(func->entry_block != NULL);

    // Clean up
    free_ast(ast);
    free_zir_module(module);
    printf("✓ Simple function translation tests passed\n");
}

void test_control_flow_translation(void)
{
    printf("\nTesting control flow translation...\n");

    // Create test AST
    ASTNode *ast = create_test_ast_control_flow();

    // Translate to ZIR
    ZIRModule *module = translate_ast_to_zir(ast);
    assert(module != NULL);
    assert(module->func_count == 2); // main + max function

    // Verify function
    ZIRFunction *func = NULL;
    for (int i = 0; i < module->func_count; i++)
    {
        if (strcmp(module->functions[i]->name, "max") == 0)
        {
            func = module->functions[i];
            break;
        }
    }
    assert(func != NULL);
    assert(func->param_count == 2);
    assert(func->return_type->kind == TYPE_I32);

    // Verify that we have the correct number of blocks (entry, then, else, merge)
    assert(func->block_count >= 4);

    // Verify that blocks are properly linked
    assert(func->entry_block != NULL);
    assert(func->entry_block->next != NULL);

    // Clean up
    free_ast(ast);
    free_zir_module(module);
    printf("✓ Control flow translation tests passed\n");
}

void test_return_statement_translation(void)
{
    printf("\nTesting return statement translation...\n");

    // Test void return
    ASTNode *void_return = create_return_stmt(NULL);
    ASTNode **void_stmts = malloc(sizeof(ASTNode *));
    void_stmts[0] = void_return;
    ASTNode *void_body = create_block(void_stmts, 1);
    ASTNode *void_func = create_func_def("void_func", NULL, 0, "void", void_body);

    // Create a block containing the void function
    ASTNode **void_block_stmts = malloc(sizeof(ASTNode *));
    void_block_stmts[0] = void_func;
    ASTNode *void_block = create_block(void_block_stmts, 1);

    ZIRModule *void_module = translate_ast_to_zir(void_block);
    assert(void_module != NULL);
    assert(void_module->func_count == 2); // main + void_func

    // Find the void function
    ZIRFunction *void_zir_func = NULL;
    for (int i = 0; i < void_module->func_count; i++)
    {
        if (strcmp(void_module->functions[i]->name, "void_func") == 0)
        {
            void_zir_func = void_module->functions[i];
            break;
        }
    }
    assert(void_zir_func != NULL);
    assert(void_zir_func->return_type->kind == TYPE_VOID);

    // Test value return
    ASTNode *value_return = create_return_stmt(create_literal("42"));
    ASTNode **value_stmts = malloc(sizeof(ASTNode *));
    value_stmts[0] = value_return;
    ASTNode *value_body = create_block(value_stmts, 1);
    ASTNode *value_func = create_func_def("value_func", NULL, 0, "i32", value_body);

    // Create a block containing the value function
    ASTNode **value_block_stmts = malloc(sizeof(ASTNode *));
    value_block_stmts[0] = value_func;
    ASTNode *value_block = create_block(value_block_stmts, 1);

    ZIRModule *value_module = translate_ast_to_zir(value_block);
    assert(value_module != NULL);
    assert(value_module->func_count == 2); // main + value_func

    // Find the value function
    ZIRFunction *value_zir_func = NULL;
    for (int i = 0; i < value_module->func_count; i++)
    {
        if (strcmp(value_module->functions[i]->name, "value_func") == 0)
        {
            value_zir_func = value_module->functions[i];
            break;
        }
    }
    assert(value_zir_func != NULL);
    assert(value_zir_func->return_type->kind == TYPE_I32);

    // Clean up
    free_ast(void_block);
    free_ast(value_block);
    free_zir_module(void_module);
    free_zir_module(value_module);
    printf("✓ Return statement translation tests passed\n");
}

int main(void)
{
    printf("Running AST to ZIR translation tests...\n");

    test_simple_function_translation();
    test_control_flow_translation();
    test_return_statement_translation();

    printf("\nAll AST to ZIR translation tests passed! ✓\n");
    return 0;
}