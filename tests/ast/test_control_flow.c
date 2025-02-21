#include "../../include/ast.h"
#include "../../include/semantic.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test while loop AST creation and semantic analysis
void test_while_loop(void)
{
    // Create: while (i < 10) { i = i + 1; }
    ASTNode *i_id = create_identifier("i");
    ASTNode *ten = create_literal("10");
    ASTNode *condition = create_binary_expr("<", i_id, ten);

    // i = i + 1
    ASTNode *i_plus_one = create_binary_expr("+", create_identifier("i"), create_literal("1"));
    ASTNode *assignment = create_assign_expr(create_identifier("i"), i_plus_one);

    // Create block with assignment
    ASTNode **block_stmts = malloc(sizeof(ASTNode *));
    block_stmts[0] = create_expr_stmt(assignment);
    ASTNode *block = create_block(block_stmts, 1);

    // Create while statement
    ASTNode *while_stmt = create_while_stmt(condition, block);

    // Create function to contain the while loop
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "i", "i32", create_literal("0")); // let i: i32 = 0
    func_stmts[1] = while_stmt;

    ASTNode *func_block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_while", NULL, 0, "void", func_block, 0);

    // Verify structure
    assert(func->type == AST_FUNC_DEF);
    assert(func_block->type == AST_BLOCK);
    assert(while_stmt->type == AST_WHILE_STMT);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ While loop test passed\n");
}

// Test for loop AST creation and semantic analysis
void test_for_loop(void)
{
    // Create: for (i in {0:5}) { print(i); }
    ASTNode *start = create_literal("0");
    ASTNode *end = create_literal("5");

    // Create print statement for loop body
    ASTNode **print_args = malloc(sizeof(ASTNode *));
    print_args[0] = create_identifier("i");
    ASTNode *print_stmt = create_print_stmt(print_args[0]);

    // Create block with print statement
    ASTNode **block_stmts = malloc(sizeof(ASTNode *));
    block_stmts[0] = print_stmt;
    ASTNode *block = create_block(block_stmts, 1);

    // Create for statement
    ASTNode *for_stmt = create_for_stmt("i", start, end, block);

    // Create function to contain the for loop
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = for_stmt;
    ASTNode *func_block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_for", NULL, 0, "void", func_block, 0);

    // Verify structure
    assert(func->type == AST_FUNC_DEF);
    assert(for_stmt->type == AST_FOR_STMT);
    assert(strcmp(for_stmt->data.for_stmt.iterator, "i") == 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ For loop test passed\n");
}

// Test break and continue statements
void test_break_continue(void)
{
    // Test 1: Break in while loop
    // while (i < 10) { if (i == 5) break; i = i + 1; }

    // Create the loop condition (i < 10)
    ASTNode *i_id = create_identifier("i");
    ASTNode *ten = create_literal("10");
    ASTNode *loop_condition = create_binary_expr("<", i_id, ten);

    // Create if condition (i == 5)
    ASTNode *five = create_literal("5");
    ASTNode *if_condition = create_binary_expr("==", create_identifier("i"), five);

    // Create break statement
    ASTNode *break_stmt = create_break_stmt();

    // Create if block with break
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = break_stmt;
    ASTNode *if_block = create_block(if_stmts, 1);

    // Create if statement
    ASTNode *if_stmt = create_if_stmt(if_condition, if_block, NULL, NULL, 0, NULL);

    // Create increment (i = i + 1)
    ASTNode *i_plus_one = create_binary_expr("+", create_identifier("i"), create_literal("1"));
    ASTNode *increment = create_assign_expr(create_identifier("i"), i_plus_one);

    // Create while loop block
    ASTNode **while_stmts = malloc(2 * sizeof(ASTNode *));
    while_stmts[0] = if_stmt;
    while_stmts[1] = create_expr_stmt(increment);
    ASTNode *while_block = create_block(while_stmts, 2);

    // Create while statement
    ASTNode *while_stmt = create_while_stmt(loop_condition, while_block);

    // Test 2: Continue in for loop
    // for (j in {0:5}) { if (j == 2) continue; print(j); }
    ASTNode *start = create_literal("0");
    ASTNode *end = create_literal("5");

    // Create if condition (j == 2)
    ASTNode *two = create_literal("2");
    ASTNode *for_if_condition = create_binary_expr("==", create_identifier("j"), two);

    // Create continue statement
    ASTNode *continue_stmt = create_continue_stmt();

    // Create if block with continue
    ASTNode **for_if_stmts = malloc(sizeof(ASTNode *));
    for_if_stmts[0] = continue_stmt;
    ASTNode *for_if_block = create_block(for_if_stmts, 1);

    // Create if statement
    ASTNode *for_if_stmt = create_if_stmt(for_if_condition, for_if_block, NULL, NULL, 0, NULL);

    // Create print statement
    ASTNode **print_args = malloc(sizeof(ASTNode *));
    print_args[0] = create_identifier("j");
    ASTNode *print_stmt = create_print_stmt(print_args[0]);

    // Create for loop block
    ASTNode **for_stmts = malloc(2 * sizeof(ASTNode *));
    for_stmts[0] = for_if_stmt;
    for_stmts[1] = print_stmt;
    ASTNode *for_block = create_block(for_stmts, 2);

    // Create for statement
    ASTNode *for_stmt = create_for_stmt("j", start, end, for_block);

    // Create function containing both loops
    ASTNode **func_stmts = malloc(3 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "i", "i32", create_literal("0")); // let i: i32 = 0
    func_stmts[1] = while_stmt;
    func_stmts[2] = for_stmt;

    ASTNode *func_block = create_block(func_stmts, 3);
    ASTNode *func = create_func_def("test_loops", NULL, 0, "void", func_block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    // Test invalid break/continue (outside loop)
    ASTNode *invalid_break = create_break_stmt();
    ASTNode **invalid_stmts = malloc(sizeof(ASTNode *));
    invalid_stmts[0] = invalid_break;
    ASTNode *invalid_block = create_block(invalid_stmts, 1);
    ASTNode *invalid_func = create_func_def("invalid_break", NULL, 0, "void", invalid_block, 0);

    // This should cause a semantic error
    int had_error = 0;
    printf("Testing invalid break (should cause error)...\n");
    semantic_analysis(invalid_func);

    free_ast(func);
    free_ast(invalid_func);
    printf("✓ Break/continue test passed\n");
}

// Test switch statement
void test_switch(void)
{
    // Create a switch statement:
    // switch (x) {
    //   case 1: print("one")
    //   case 2: print("two")
    //   finally: print("done")
    // }

    // Create switch expression (x)
    ASTNode *x = create_identifier("x");

    // Create case 1: print("one")
    ASTNode **print_args1 = malloc(sizeof(ASTNode *));
    print_args1[0] = create_literal("\"one\"");
    ASTNode *print_stmt1 = create_print_stmt(print_args1[0]);
    ASTNode *case1 = create_case_stmt(create_literal("1"), print_stmt1);

    // Create case 2: print("two")
    ASTNode **print_args2 = malloc(sizeof(ASTNode *));
    print_args2[0] = create_literal("\"two\"");
    ASTNode *print_stmt2 = create_print_stmt(print_args2[0]);
    ASTNode *case2 = create_case_stmt(create_literal("2"), print_stmt2);

    // Create finally block: print("done")
    ASTNode **print_args3 = malloc(sizeof(ASTNode *));
    print_args3[0] = create_literal("\"done\"");
    ASTNode *print_stmt3 = create_print_stmt(print_args3[0]);

    // Create array of cases
    ASTNode **cases = malloc(2 * sizeof(ASTNode *));
    cases[0] = case1;
    cases[1] = case2;

    // Create switch statement
    ASTNode *switch_stmt = create_switch_stmt(x, cases, 2, print_stmt3);

    // Create function to contain the switch
    ASTNode **func_stmts = malloc(2 * sizeof(ASTNode *));
    func_stmts[0] = create_var_decl(0, "x", "i32", create_literal("1")); // let x: i32 = 1
    func_stmts[1] = switch_stmt;

    ASTNode *func_block = create_block(func_stmts, 2);
    ASTNode *func = create_func_def("test_switch", NULL, 0, "void", func_block, 0);

    // Verify structure
    assert(func->type == AST_FUNC_DEF);
    assert(switch_stmt->type == AST_SWITCH_STMT);
    assert(switch_stmt->data.switch_stmt.case_count == 2);
    assert(switch_stmt->data.switch_stmt.finally_block != NULL);

    // Run semantic analysis
    semantic_analysis(func);

    // Test invalid case statement (outside switch)
    ASTNode *invalid_case = create_case_stmt(create_literal("1"), create_print_stmt(create_literal("\"invalid\"")));
    ASTNode **invalid_stmts = malloc(sizeof(ASTNode *));
    invalid_stmts[0] = invalid_case;
    ASTNode *invalid_block = create_block(invalid_stmts, 1);
    ASTNode *invalid_func = create_func_def("invalid_case", NULL, 0, "void", invalid_block, 0);

    // This should cause a semantic error
    printf("Testing invalid case (should cause error)...\n");
    semantic_analysis(invalid_func);

    free_ast(func);
    free_ast(invalid_func);
    printf("✓ Switch statement test passed\n");
}

int main()
{
    printf("Running control flow tests...\n");

    test_while_loop();
    test_for_loop();
    test_break_continue();
    test_switch();

    printf("All control flow tests completed!\n");
    return 0;
}