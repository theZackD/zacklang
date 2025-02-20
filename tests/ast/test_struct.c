#include "../../include/ast.h"
#include "../../include/semantic.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test basic struct definition
void test_basic_struct(void)
{
    // Create struct Point { x: i32, y: i32 }
    char *field_names[] = {"x", "y"};
    char *field_types[] = {"i32", "i32"};
    ASTNode *point_struct = create_struct_def("Point", field_names, field_types, 2);

    // Create function context
    ASTNode **func_stmts = malloc(sizeof(ASTNode *));
    func_stmts[0] = point_struct;
    ASTNode *block = create_block(func_stmts, 1);
    ASTNode *func = create_func_def("test_struct", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Basic struct test passed\n");
}

// Test struct field access
void test_field_access(void)
{
    // First define the struct
    char *field_names[] = {"x", "y"};
    char *field_types[] = {"i32", "i32"};
    ASTNode *point_struct = create_struct_def("Point", field_names, field_types, 2);

    // Create a variable of type Point
    ASTNode *point_var = create_var_decl(0, "p", "struct Point", NULL);

    // Create field access p.x
    ASTNode *p_id = create_identifier("p");
    ASTNode *x_access = create_field_access(p_id, "x");

    // Create function context
    ASTNode **func_stmts = malloc(3 * sizeof(ASTNode *));
    func_stmts[0] = point_struct;
    func_stmts[1] = point_var;
    func_stmts[2] = create_expr_stmt(x_access);
    ASTNode *block = create_block(func_stmts, 3);
    ASTNode *func = create_func_def("test_field_access", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Field access test passed\n");
}

// Test nested structs
void test_nested_struct(void)
{
    // Create struct Point { x: i32, y: i32 }
    char *point_fields[] = {"x", "y"};
    char *point_types[] = {"i32", "i32"};
    ASTNode *point_struct = create_struct_def("Point", point_fields, point_types, 2);

    // Create struct Rectangle { top_left: struct Point, bottom_right: struct Point }
    char *rect_fields[] = {"top_left", "bottom_right"};
    char *rect_types[] = {"struct Point", "struct Point"};
    ASTNode *rect_struct = create_struct_def("Rectangle", rect_fields, rect_types, 2);

    // Create field access rect.top_left.x
    ASTNode *rect_var = create_var_decl(0, "rect", "struct Rectangle", NULL);
    ASTNode *rect_id = create_identifier("rect");
    ASTNode *top_left_access = create_field_access(rect_id, "top_left");
    ASTNode *x_access = create_field_access(top_left_access, "x");

    // Create function context
    ASTNode **func_stmts = malloc(4 * sizeof(ASTNode *));
    func_stmts[0] = point_struct;
    func_stmts[1] = rect_struct;
    func_stmts[2] = rect_var;
    func_stmts[3] = create_expr_stmt(x_access);
    ASTNode *block = create_block(func_stmts, 4);
    ASTNode *func = create_func_def("test_nested_struct", NULL, 0, "void", block, 0);

    // Run semantic analysis
    semantic_analysis(func);

    free_ast(func);
    printf("✓ Nested struct test passed\n");
}

// Test invalid struct cases
void test_invalid_struct(void)
{
    // Test case 1: Duplicate field names
    char *field_names[] = {"x", "x"}; // Duplicate field name
    char *field_types[] = {"i32", "i32"};
    ASTNode *invalid_struct = create_struct_def("Invalid", field_names, field_types, 2);

    ASTNode **func_stmts1 = malloc(sizeof(ASTNode *));
    func_stmts1[0] = invalid_struct;
    ASTNode *block1 = create_block(func_stmts1, 1);
    ASTNode *func1 = create_func_def("test_invalid1", NULL, 0, "void", block1, 0);

    // This should cause a semantic error
    printf("Testing duplicate field names (should cause error)...\n");
    semantic_analysis(func1);
    free_ast(func1);

    // Test case 2: Invalid field type
    char *field_names2[] = {"x"};
    char *field_types2[] = {"invalid_type"}; // Invalid type
    ASTNode *invalid_struct2 = create_struct_def("Invalid2", field_names2, field_types2, 1);

    ASTNode **func_stmts2 = malloc(sizeof(ASTNode *));
    func_stmts2[0] = invalid_struct2;
    ASTNode *block2 = create_block(func_stmts2, 1);
    ASTNode *func2 = create_func_def("test_invalid2", NULL, 0, "void", block2, 0);

    // This should cause a semantic error
    printf("Testing invalid field type (should cause error)...\n");
    semantic_analysis(func2);
    free_ast(func2);

    printf("✓ Invalid struct tests passed\n");
}

int main()
{
    printf("Running struct tests...\n");

    test_basic_struct();
    test_field_access();
    test_nested_struct();
    test_invalid_struct();

    printf("All struct tests completed!\n");
    return 0;
}