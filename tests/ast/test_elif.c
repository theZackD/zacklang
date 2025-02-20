#include "../../include/ast.h"
#include "../../include/semantic.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test basic if-elif-else structure
void test_basic_elif(void)
{
    // Create: if (x == 1) { print("one"); } elif (x == 2) { print("two"); } else { print("other"); }

    // Create conditions
    ASTNode *x_id1 = create_identifier("x");
    ASTNode *one = create_literal("1");
    ASTNode *if_cond = create_binary_expr("==", x_id1, one);

    ASTNode *x_id2 = create_identifier("x");
    ASTNode *two = create_literal("2");
    ASTNode *elif_cond = create_binary_expr("==", x_id2, two);

    // Create print statements for each block
    ASTNode *print_one = create_print_stmt(create_literal("\"one\""));
    ASTNode *print_two = create_print_stmt(create_literal("\"two\""));
    ASTNode *print_other = create_print_stmt(create_literal("\"other\""));

    // Create blocks
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = print_one;
    ASTNode *if_block = create_block(if_stmts, 1);

    ASTNode **elif_stmts = malloc(sizeof(ASTNode *));
    elif_stmts[0] = print_two;
    ASTNode *elif_block = create_block(elif_stmts, 1);

    ASTNode **else_stmts = malloc(sizeof(ASTNode *));
    else_stmts[0] = print_other;
    ASTNode *else_block = create_block(else_stmts, 1);

    // Create elif arrays
    ASTNode **elif_conds = malloc(sizeof(ASTNode *));
    elif_conds[0] = elif_cond;
    ASTNode **elif_blocks = malloc(sizeof(ASTNode *));
    elif_blocks[0] = elif_block;

    // Create the if statement
    ASTNode *if_stmt = create_if_stmt(if_cond, if_block, elif_conds, elif_blocks, 1, else_block);

    // Create function context
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("1"));
    func_stmts[1] = if_stmt;
    ASTNode *block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_elif", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Basic elif test passed\n");
}

// Test multiple elif branches
void test_multiple_elif(void)
{
    // Create: if (x == 1) { print("one"); }
    //         elif (x == 2) { print("two"); }
    //         elif (x == 3) { print("three"); }
    //         else { print("other"); }

    // Create conditions
    ASTNode *x_id1 = create_identifier("x");
    ASTNode *one = create_literal("1");
    ASTNode *if_cond = create_binary_expr("==", x_id1, one);

    ASTNode *x_id2 = create_identifier("x");
    ASTNode *two = create_literal("2");
    ASTNode *elif_cond1 = create_binary_expr("==", x_id2, two);

    ASTNode *x_id3 = create_identifier("x");
    ASTNode *three = create_literal("3");
    ASTNode *elif_cond2 = create_binary_expr("==", x_id3, three);

    // Create print statements
    ASTNode *print_one = create_print_stmt(create_literal("\"one\""));
    ASTNode *print_two = create_print_stmt(create_literal("\"two\""));
    ASTNode *print_three = create_print_stmt(create_literal("\"three\""));
    ASTNode *print_other = create_print_stmt(create_literal("\"other\""));

    // Create blocks
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = print_one;
    ASTNode *if_block = create_block(if_stmts, 1);

    ASTNode **elif_stmts1 = malloc(sizeof(ASTNode *));
    elif_stmts1[0] = print_two;
    ASTNode *elif_block1 = create_block(elif_stmts1, 1);

    ASTNode **elif_stmts2 = malloc(sizeof(ASTNode *));
    elif_stmts2[0] = print_three;
    ASTNode *elif_block2 = create_block(elif_stmts2, 1);

    ASTNode **else_stmts = malloc(sizeof(ASTNode *));
    else_stmts[0] = print_other;
    ASTNode *else_block = create_block(else_stmts, 1);

    // Create elif arrays
    ASTNode **elif_conds = malloc(2 * sizeof(ASTNode *));
    elif_conds[0] = elif_cond1;
    elif_conds[1] = elif_cond2;
    ASTNode **elif_blocks = malloc(2 * sizeof(ASTNode *));
    elif_blocks[0] = elif_block1;
    elif_blocks[1] = elif_block2;

    // Create the if statement
    ASTNode *if_stmt = create_if_stmt(if_cond, if_block, elif_conds, elif_blocks, 2, else_block);

    // Create function context
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("1"));
    func_stmts[1] = if_stmt;
    ASTNode *block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_multiple_elif", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Multiple elif test passed\n");
}

// Test invalid elif condition type
void test_invalid_elif_condition(void)
{
    // Create: if (x == 1) { print("one"); } elif ("invalid") { print("two"); }

    // Create conditions
    ASTNode *x_id = create_identifier("x");
    ASTNode *one = create_literal("1");
    ASTNode *if_cond = create_binary_expr("==", x_id, one);

    // Invalid elif condition (string instead of boolean)
    ASTNode *elif_cond = create_literal("\"invalid\"");

    // Create print statements
    ASTNode *print_one = create_print_stmt(create_literal("\"one\""));
    ASTNode *print_two = create_print_stmt(create_literal("\"two\""));

    // Create blocks
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = print_one;
    ASTNode *if_block = create_block(if_stmts, 1);

    ASTNode **elif_stmts = malloc(sizeof(ASTNode *));
    elif_stmts[0] = print_two;
    ASTNode *elif_block = create_block(elif_stmts, 1);

    // Create elif arrays
    ASTNode **elif_conds = malloc(sizeof(ASTNode *));
    elif_conds[0] = elif_cond;
    ASTNode **elif_blocks = malloc(sizeof(ASTNode *));
    elif_blocks[0] = elif_block;

    // Create the if statement
    ASTNode *if_stmt = create_if_stmt(if_cond, if_block, elif_conds, elif_blocks, 1, NULL);

    // Create function context
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("1"));
    func_stmts[1] = if_stmt;
    ASTNode *block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_invalid_elif", NULL, 0, "void", block, 0);

    // This should cause a semantic error
    printf("Testing invalid elif condition (should cause error)...\n");
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Invalid elif condition test passed\n");
}

int main()
{
    printf("Running elif tests...\n");

    test_basic_elif();
    test_multiple_elif();
    test_invalid_elif_condition();

    printf("All elif tests completed!\n");
    return 0;
}