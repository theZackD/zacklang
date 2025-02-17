#include "include/ast.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

void test_var_declaration()
{
    printf("\nTesting variable declaration...\n");

    // Test regular variable declaration
    ASTNode *init = create_literal("42");
    ASTNode *var = create_var_decl(0, "x", "i32", init);

    assert(var->type == AST_VAR_DECL);
    assert(var->data.var_decl.is_const == 0);
    assert(strcmp(var->data.var_decl.identifier, "x") == 0);
    assert(strcmp(var->data.var_decl.type_annotation, "i32") == 0);
    assert(var->data.var_decl.initializer->type == AST_LITERAL);
    assert(strcmp(var->data.var_decl.initializer->data.literal.value, "42") == 0);

    free_ast(var);
    printf("✓ Variable declaration test passed\n");
}

void test_binary_expression()
{
    printf("\nTesting binary expression...\n");

    // Test 2 + 3 * 4
    ASTNode *num2 = create_literal("2");
    ASTNode *num3 = create_literal("3");
    ASTNode *num4 = create_literal("4");

    ASTNode *mult = create_binary_expr("*", num3, num4);
    ASTNode *add = create_binary_expr("+", num2, mult);

    assert(add->type == AST_BINARY_EXPR);
    assert(strcmp(add->data.binary_expr.op, "+") == 0);
    assert(add->data.binary_expr.left->type == AST_LITERAL);
    assert(add->data.binary_expr.right->type == AST_BINARY_EXPR);

    free_ast(add);
    printf("✓ Binary expression test passed\n");
}

void test_if_statement()
{
    printf("\nTesting if statement...\n");

    // Test: if (x > 0) { print(x); } else { print(-x); }
    ASTNode *x_id = create_identifier("x");
    ASTNode *zero = create_literal("0");
    ASTNode *condition = create_binary_expr(">", x_id, zero);

    // if block
    ASTNode *print_x = create_print_stmt(create_identifier("x"));
    ASTNode **if_stmts = malloc(sizeof(ASTNode *));
    if_stmts[0] = print_x;
    ASTNode *if_block = create_block(if_stmts, 1);

    // else block
    ASTNode *neg_x = create_unary_expr("-", create_identifier("x"));
    ASTNode *print_neg_x = create_print_stmt(neg_x);
    ASTNode **else_stmts = malloc(sizeof(ASTNode *));
    else_stmts[0] = print_neg_x;
    ASTNode *else_block = create_block(else_stmts, 1);

    ASTNode *if_stmt = create_if_stmt(condition, if_block, else_block);

    assert(if_stmt->type == AST_IF_STMT);
    assert(if_stmt->data.if_stmt.condition->type == AST_BINARY_EXPR);
    assert(if_stmt->data.if_stmt.if_block->type == AST_BLOCK);
    assert(if_stmt->data.if_stmt.else_block->type == AST_BLOCK);

    free_ast(if_stmt);
    printf("✓ If statement test passed\n");
}

void test_function_definition()
{
    printf("\nTesting function definition...\n");

    // Test: fn add(a: i32, b: i32): i32 { return a + b; }
    ASTNode *param_a = create_var_decl(0, "a", "i32", NULL);
    ASTNode *param_b = create_var_decl(0, "b", "i32", NULL);
    ASTNode **params = malloc(2 * sizeof(ASTNode *));
    params[0] = param_a;
    params[1] = param_b;

    ASTNode *a_id = create_identifier("a");
    ASTNode *b_id = create_identifier("b");
    ASTNode *add_expr = create_binary_expr("+", a_id, b_id);
    ASTNode **body_stmts = malloc(sizeof(ASTNode *));
    body_stmts[0] = add_expr;
    ASTNode *body = create_block(body_stmts, 1);

    ASTNode *func = create_func_def("add", params, 2, "i32", body);

    assert(func->type == AST_FUNC_DEF);
    assert(strcmp(func->data.func_def.name, "add") == 0);
    assert(func->data.func_def.param_count == 2);
    assert(strcmp(func->data.func_def.return_type, "i32") == 0);

    free_ast(func);
    printf("✓ Function definition test passed\n");
}

void test_loop_statements()
{
    printf("\nTesting loop statements...\n");

    // Test while loop: while (x > 0) { x = x - 1; }
    ASTNode *x_id = create_identifier("x");
    ASTNode *zero = create_literal("0");
    ASTNode *condition = create_binary_expr(">", x_id, zero);

    ASTNode *x_minus_one = create_binary_expr("-", create_identifier("x"), create_literal("1"));
    ASTNode *assignment = create_assign_expr(create_identifier("x"), x_minus_one);
    ASTNode **while_stmts = malloc(sizeof(ASTNode *));
    while_stmts[0] = assignment;
    ASTNode *while_body = create_block(while_stmts, 1);

    ASTNode *while_stmt = create_while_stmt(condition, while_body);

    assert(while_stmt->type == AST_WHILE_STMT);
    assert(while_stmt->data.while_stmt.condition->type == AST_BINARY_EXPR);
    assert(while_stmt->data.while_stmt.block->type == AST_BLOCK);

    free_ast(while_stmt);

    // Test for loop: for (i in 0..10) { print(i); }
    ASTNode *start = create_literal("0");
    ASTNode *end = create_literal("10");
    ASTNode *print_i = create_print_stmt(create_identifier("i"));
    ASTNode **for_stmts = malloc(sizeof(ASTNode *));
    for_stmts[0] = print_i;
    ASTNode *for_body = create_block(for_stmts, 1);

    ASTNode *for_stmt = create_for_stmt("i", start, end, for_body);

    assert(for_stmt->type == AST_FOR_STMT);
    assert(strcmp(for_stmt->data.for_stmt.iterator, "i") == 0);
    assert(for_stmt->data.for_stmt.start_expr->type == AST_LITERAL);
    assert(for_stmt->data.for_stmt.end_expr->type == AST_LITERAL);

    free_ast(for_stmt);
    printf("✓ Loop statements test passed\n");
}

int main()
{
    printf("Running AST tests...\n");

    test_var_declaration();
    test_binary_expression();
    test_if_statement();
    test_function_definition();
    test_loop_statements();

    printf("\nAll AST tests passed! ✓\n");
    return 0;
}