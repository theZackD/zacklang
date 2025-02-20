#include "../../include/ast.h"
#include "../../include/comptime.h"
#include "../../include/static_types.h"
#include "../test_utils.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Test basic literal evaluation
void test_literal_evaluation(void)
{
    // Integer literal
    ASTNode *int_lit = create_literal("42");
    ComptimeValue *int_val = evaluate_comptime_expr(int_lit);
    assert(int_val->type->kind == TYPE_I32);
    assert(int_val->value.i_val == 42);
    printf("✓ Integer literal evaluation passed\n");
    free_comptime_value(int_val);
    free_ast(int_lit);

    // Float literal
    ASTNode *float_lit = create_literal("3.14");
    ComptimeValue *float_val = evaluate_comptime_expr(float_lit);
    assert(float_val->type->kind == TYPE_F64);
    assert(float_val->value.f_val == 3.14);
    printf("✓ Float literal evaluation passed\n");
    free_comptime_value(float_val);
    free_ast(float_lit);

    // Boolean literal
    ASTNode *bool_lit = create_literal("true");
    ComptimeValue *bool_val = evaluate_comptime_expr(bool_lit);
    assert(bool_val->type->kind == TYPE_BOOL);
    assert(bool_val->value.b_val == true);
    printf("✓ Boolean literal evaluation passed\n");
    free_comptime_value(bool_val);
    free_ast(bool_lit);

    // String literal
    ASTNode *str_lit = create_literal("\"hello\"");
    ComptimeValue *str_val = evaluate_comptime_expr(str_lit);
    assert(str_val->type->kind == TYPE_STRING);
    assert(strcmp(str_val->value.s_val, "hello") == 0);
    printf("✓ String literal evaluation passed\n");
    free_comptime_value(str_val);
    free_ast(str_lit);
}

// Test const variable evaluation
void test_const_evaluation(void)
{
    // Create a const variable declaration
    ASTNode *const_decl = create_var_decl(1, "x", "i32", create_literal("42"));

    // Create a reference to the const variable
    ASTNode *const_ref = create_identifier("x");

    // Create a symbol table and add the const variable
    SymbolTable *table = create_symbol_table(NULL);
    add_symbol_with_node(table, "x", "i32", const_decl);

    // Evaluate the const reference
    ComptimeValue *result = evaluate_comptime_expr_with_symbols(const_ref, table);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 42);
    printf("✓ Basic const evaluation passed\n");
    free_comptime_value(result);

    // Test const in binary expression
    ASTNode *add = create_binary_expr("+", const_ref, create_literal("8"));
    result = evaluate_comptime_expr_with_symbols(add, table);
    assert(result != NULL);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 50);
    printf("✓ Const in binary expression passed\n");
    free_comptime_value(result);

    // Test const string
    ASTNode *str_decl = create_var_decl(1, "msg", "string", create_literal("\"hello\""));
    add_symbol_with_node(table, "msg", "string", str_decl);
    ASTNode *str_ref = create_identifier("msg");
    result = evaluate_comptime_expr_with_symbols(str_ref, table);
    assert(result != NULL);
    assert(result->type->kind == TYPE_STRING);
    assert(strcmp(result->value.s_val, "hello") == 0);
    printf("✓ Const string evaluation passed\n");
    free_comptime_value(result);

    // Cleanup
    free_ast(add);
    free_ast(str_decl);
    free_ast(const_decl);
    destroy_symbol_table(table);
}

// Test arithmetic operations
void test_arithmetic_operations(void)
{
    // Addition: 2 + 3
    ASTNode *two = create_literal("2");
    ASTNode *three = create_literal("3");
    ASTNode *add = create_binary_expr("+", two, three);
    ComptimeValue *result = evaluate_comptime_expr(add);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 5);
    printf("✓ Addition evaluation passed\n");
    free_comptime_value(result);
    free_ast(add);

    // Mixed type: 2.0 + 3.5
    ASTNode *float_two = create_literal("2.0");
    ASTNode *float_three = create_literal("3.5");
    ASTNode *mixed_add = create_binary_expr("+", float_two, float_three);
    result = evaluate_comptime_expr(mixed_add);
    assert(result->type->kind == TYPE_F64);
    assert(result->value.f_val == 5.5);
    printf("✓ Mixed type addition evaluation passed\n");
    free_comptime_value(result);
    free_ast(mixed_add);

    // Power operator: 2 ** 3
    ASTNode *base = create_literal("2");
    ASTNode *exp = create_literal("3");
    ASTNode *power = create_binary_expr("**", base, exp);
    result = evaluate_comptime_expr(power);
    assert(result->type->kind == TYPE_I32);
    assert(result->value.i_val == 8);
    printf("✓ Power operator evaluation passed\n");
    free_comptime_value(result);
    free_ast(power);
}

// Test comparison operations
void test_comparison_operations(void)
{
    // Equal: 2 == 2
    ASTNode *two1 = create_literal("2");
    ASTNode *two2 = create_literal("2");
    ASTNode *eq = create_binary_expr("==", two1, two2);
    ComptimeValue *result = evaluate_comptime_expr(eq);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Equality comparison passed\n");
    free_comptime_value(result);
    free_ast(eq);

    // Less than: 2 < 3
    ASTNode *two = create_literal("2");
    ASTNode *three = create_literal("3");
    ASTNode *lt = create_binary_expr("<", two, three);
    result = evaluate_comptime_expr(lt);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Less than comparison passed\n");
    free_comptime_value(result);
    free_ast(lt);
}

// Test logical operations
void test_logical_operations(void)
{
    // AND: true and false
    ASTNode *true_lit1 = create_literal("true");
    ASTNode *false_lit1 = create_literal("false");
    ASTNode *and_expr = create_binary_expr("and", true_lit1, false_lit1);
    ComptimeValue *result = evaluate_comptime_expr(and_expr);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == false);
    printf("✓ Logical AND evaluation passed\n");
    free_comptime_value(result);
    free_ast(and_expr);

    // OR: true or false
    ASTNode *true_lit2 = create_literal("true");
    ASTNode *false_lit2 = create_literal("false");
    ASTNode *or_expr = create_binary_expr("or", true_lit2, false_lit2);
    result = evaluate_comptime_expr(or_expr);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == true);
    printf("✓ Logical OR evaluation passed\n");
    free_comptime_value(result);
    free_ast(or_expr);

    // NOT: not true
    ASTNode *true_lit3 = create_literal("true");
    ASTNode *not_expr = create_unary_expr("not", true_lit3);
    result = evaluate_comptime_expr(not_expr);
    assert(result->type->kind == TYPE_BOOL);
    assert(result->value.b_val == false);
    printf("✓ Logical NOT evaluation passed\n");
    free_comptime_value(result);
    free_ast(not_expr);
}

// Test error cases
void test_error_cases(void)
{
    // Division by zero
    ASTNode *one = create_literal("1");
    ASTNode *zero = create_literal("0");
    ASTNode *div_zero = create_binary_expr("/", one, zero);
    ComptimeValue *result = evaluate_comptime_expr(div_zero);
    assert(result == NULL);
    printf("✓ Division by zero error handling passed\n");
    free_ast(div_zero);

    // Invalid operation type
    ASTNode *str = create_literal("\"hello\"");
    ASTNode *one_new = create_literal("1");
    ASTNode *invalid_op = create_binary_expr("+", one_new, str);
    result = evaluate_comptime_expr(invalid_op);
    assert(result == NULL);
    printf("✓ Invalid operation type error handling passed\n");
    free_ast(invalid_op);
}

// Test array operations
void test_array_operations(void)
{
    // Test array literal creation
    ASTNode **elements = malloc(3 * sizeof(ASTNode *));
    elements[0] = create_literal("1");
    elements[1] = create_literal("2");
    elements[2] = create_literal("3");
    ASTNode *array_lit = create_array_literal(elements, 3);

    // Create array type with i32 elements
    Type *element_type = create_type(TYPE_I32);
    element_type->is_comptime = true;
    array_lit->data.array_literal.element_type = element_type;

    ComptimeValue *result = evaluate_comptime_expr(array_lit);
    assert(result != NULL);
    assert(result->type->kind == TYPE_ARRAY);
    assert(result->value.array_val.length == 3);
    assert(result->value.array_val.elements[0]->value.i_val == 1);
    assert(result->value.array_val.elements[1]->value.i_val == 2);
    assert(result->value.array_val.elements[2]->value.i_val == 3);
    printf("✓ Array literal evaluation passed\n");

    // Test array indexing
    ASTNode *index = create_literal("1");
    ASTNode *array_index = create_array_index(array_lit, index);
    ComptimeValue *index_result = evaluate_comptime_expr(array_index);
    assert(index_result != NULL);
    assert(index_result->type->kind == TYPE_I32);
    assert(index_result->value.i_val == 2);
    printf("✓ Array indexing passed\n");

    // Test array length
    ASTNode *length_call = create_func_call("len", &array_lit, 1);
    ComptimeValue *length_result = evaluate_comptime_expr(length_call);
    assert(length_result != NULL);
    assert(length_result->type->kind == TYPE_I32);
    assert(length_result->value.i_val == 3);
    printf("✓ Array length passed\n");

    // Test out of bounds error
    ASTNode *bad_index = create_literal("5");
    ASTNode *bad_access = create_array_index(array_lit, bad_index);
    ComptimeValue *error_result = evaluate_comptime_expr(bad_access);
    assert(error_result == NULL);
    printf("✓ Array bounds checking passed\n");

    // Test array of strings
    ASTNode **str_elements = malloc(2 * sizeof(ASTNode *));
    str_elements[0] = create_literal("\"hello\"");
    str_elements[1] = create_literal("\"world\"");
    ASTNode *str_array = create_array_literal(str_elements, 2);

    // Create array type with string elements
    Type *str_element_type = create_type(TYPE_STRING);
    str_element_type->is_comptime = true;
    str_array->data.array_literal.element_type = str_element_type;

    ComptimeValue *str_result = evaluate_comptime_expr(str_array);
    assert(str_result != NULL);
    assert(str_result->type->kind == TYPE_ARRAY);
    assert(str_result->value.array_val.length == 2);
    assert(strcmp(str_result->value.array_val.elements[0]->value.s_val, "hello") == 0);
    assert(strcmp(str_result->value.array_val.elements[1]->value.s_val, "world") == 0);
    printf("✓ String array evaluation passed\n");

    // Cleanup
    free_comptime_value(result);
    free_comptime_value(index_result);
    free_comptime_value(length_result);
    free_comptime_value(str_result);

    // Free AST nodes in reverse order of creation
    free_ast(bad_access);
    free_ast(length_call);
    free_ast(array_index);
    free_ast(str_array);
    free_ast(array_lit);
}

int main()
{
    printf("Running comptime evaluation tests...\n");

    test_literal_evaluation();
    test_const_evaluation();
    test_arithmetic_operations();
    test_comparison_operations();
    test_logical_operations();
    test_error_cases();
    test_array_operations();

    printf("All comptime evaluation tests completed!\n");
    return 0;
}