#include "include/variable_tracking.h"
#include "include/static_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Helper function to print a ZIR value
static void print_zir_value(ZIRValue *value)
{
    if (!value)
    {
        printf("NULL");
        return;
    }

    printf("ZIRValue{kind=%d, type=%s, is_comptime=%d",
           value->kind,
           type_to_string(value->type),
           value->is_comptime);

    if (value->kind == ZIR_CONST)
    {
        printf(", value='%s'", value->data.constant.literal_value);
    }
    printf("}");
}

// Test a simple if-else scenario with variable updates
void test_if_else_scenario(void)
{
    printf("\nTesting if-else scenario with variable tracking...\n");

    // Create variable table
    VariableTable *table = create_variable_table(10);

    // Create blocks
    ZIRBlock *entry = create_zir_block("entry");
    ZIRBlock *then_block = create_zir_block("then");
    ZIRBlock *else_block = create_zir_block("else");
    ZIRBlock *merge_block = create_zir_block("merge");

    // Create initial value for 'x'
    Type *i32_type = create_type(TYPE_I32);
    ZIRValue *x_init = create_zir_const(i32_type, "10");
    int x_v0 = create_new_version(table, "x", x_init);
    assert(x_v0 == 0);

    // Create condition (x > 5)
    ZIRValue *five = create_zir_const(i32_type, "5");
    ZIRValue *cond = create_zir_binary(">", get_current_definition(table, "x"), five);

    // Create branch
    ZIRValue *branch = create_zir_branch(cond, then_block, else_block);
    zir_block_add_instr(entry, branch);

    // Then block: x = x + 1
    ZIRValue *one = create_zir_const(i32_type, "1");
    ZIRValue *then_add = create_zir_binary("+", get_current_definition(table, "x"), one);
    int x_v1 = create_new_version(table, "x", then_add);
    assert(x_v1 == 1);
    zir_block_add_instr(then_block, then_add);

    // Else block: x = x - 1
    ZIRValue *else_sub = create_zir_binary("-", get_current_definition(table, "x"), one);
    int x_v2 = create_new_version(table, "x", else_sub);
    assert(x_v2 == 2);
    zir_block_add_instr(else_block, else_sub);

    // Merge block: create phi node
    ZIRValue *incoming_values[] = {
        get_version_definition(table, "x", x_v1),
        get_version_definition(table, "x", x_v2)};
    ZIRBlock *incoming_blocks[] = {then_block, else_block};
    ZIRValue *phi = create_zir_phi(i32_type, incoming_values, incoming_blocks, 2);
    int x_v3 = create_new_version(table, "x", phi);
    assert(x_v3 == 3);
    zir_block_add_instr(merge_block, phi);

    // Verify variable versions
    printf("\nVerifying variable versions:\n");
    printf("x_v0 (initial): ");
    print_zir_value(get_version_definition(table, "x", 0));
    printf("\n");

    printf("x_v1 (then): ");
    print_zir_value(get_version_definition(table, "x", 1));
    printf("\n");

    printf("x_v2 (else): ");
    print_zir_value(get_version_definition(table, "x", 2));
    printf("\n");

    printf("x_v3 (merge): ");
    print_zir_value(get_version_definition(table, "x", 3));
    printf("\n");

    // Verify version count
    assert(get_version_count(table, "x") == 4);

    // Verify current version is the phi node
    assert(get_current_definition(table, "x") == phi);

    // Clean up
    free_zir_block(entry);
    free_zir_block(then_block);
    free_zir_block(else_block);
    free_zir_block(merge_block);
    free_type(i32_type);
    destroy_variable_table(table);

    printf("✓ If-else scenario test passed\n");
}

// Test a loop scenario with multiple iterations
void test_loop_scenario(void)
{
    printf("\nTesting loop scenario with variable tracking...\n");

    // Create variable table
    VariableTable *table = create_variable_table(10);

    // Create blocks
    ZIRBlock *entry = create_zir_block("entry");
    ZIRBlock *loop_header = create_zir_block("loop_header");
    ZIRBlock *loop_body = create_zir_block("loop_body");
    ZIRBlock *loop_exit = create_zir_block("loop_exit");

    // Initialize 'i' to 0
    Type *i32_type = create_type(TYPE_I32);
    ZIRValue *zero = create_zir_const(i32_type, "0");
    int i_v0 = create_new_version(table, "i", zero);
    assert(i_v0 == 0);

    // Create phi node for loop variable
    ZIRValue *incoming_values[] = {
        get_current_definition(table, "i"),
        NULL // Will be set after creating increment
    };
    ZIRBlock *incoming_blocks[] = {
        entry,
        loop_body};
    ZIRValue *phi = create_zir_phi(i32_type, incoming_values, incoming_blocks, 2);
    int i_v1 = create_new_version(table, "i", phi);
    assert(i_v1 == 1);
    zir_block_add_instr(loop_header, phi);

    // Create loop condition (i < 10)
    ZIRValue *ten = create_zir_const(i32_type, "10");
    ZIRValue *cond = create_zir_binary("<", get_current_definition(table, "i"), ten);
    ZIRValue *branch = create_zir_branch(cond, loop_body, loop_exit);
    zir_block_add_instr(loop_header, branch);

    // Create loop body (i = i + 1)
    ZIRValue *one = create_zir_const(i32_type, "1");
    ZIRValue *increment = create_zir_binary("+", get_current_definition(table, "i"), one);
    int i_v2 = create_new_version(table, "i", increment);
    assert(i_v2 == 2);
    zir_block_add_instr(loop_body, increment);

    // Update phi node with the increment
    incoming_values[1] = increment;
    zir_phi_add_incoming(phi, increment, loop_body);

    // Verify variable versions
    printf("\nVerifying variable versions:\n");
    printf("i_v0 (initial): ");
    print_zir_value(get_version_definition(table, "i", 0));
    printf("\n");

    printf("i_v1 (phi): ");
    print_zir_value(get_version_definition(table, "i", 1));
    printf("\n");

    printf("i_v2 (increment): ");
    print_zir_value(get_version_definition(table, "i", 2));
    printf("\n");

    // Verify version count
    assert(get_version_count(table, "i") == 3);

    // Clean up
    free_zir_block(entry);
    free_zir_block(loop_header);
    free_zir_block(loop_body);
    free_zir_block(loop_exit);
    free_type(i32_type);
    destroy_variable_table(table);

    printf("✓ Loop scenario test passed\n");
}

int main(void)
{
    printf("Running variable tracking integration tests...\n");

    test_if_else_scenario();
    test_loop_scenario();

    printf("\nAll variable tracking integration tests passed! ✓\n");
    return 0;
}