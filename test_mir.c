#include "include/mir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Helper function for string comparison
static void assert_str_eq(const char *actual, const char *expected)
{
    if (strcmp(actual, expected) != 0)
    {
        printf("Expected '%s' but got '%s'\n", expected, actual);
        assert(0);
    }
}

void test_module_creation(void)
{
    printf("\nTesting module creation...\n");

    MIRModule *module = create_mir_module();
    assert(module != NULL);
    assert(module->first_func == NULL);
    assert(module->last_func == NULL);

    free_mir_module(module);
    printf("✓ Module creation test passed\n");
}

void test_function_creation(void)
{
    printf("\nTesting function creation...\n");

    Type *return_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("main", return_type);

    assert(func != NULL);
    assert_str_eq(func->name, "main");
    assert(func->return_type == return_type);
    assert(func->reg_count == 0);
    assert(func->param_count == 0);
    assert(func->first_block == NULL);

    // Add a parameter
    Type *param_type = create_type(TYPE_F64);
    mir_function_add_param(func, "x", param_type);
    assert(func->param_count == 1);
    assert_str_eq(func->params[0].name, "x");
    assert(func->params[0].type == param_type);

    free_mir_function(func);
    free_type(return_type);
    free_type(param_type);
    printf("✓ Function creation test passed\n");
}

void test_block_creation(void)
{
    printf("\nTesting block creation...\n");

    MIRBlock *block = create_mir_block("entry");
    assert(block != NULL);
    assert_str_eq(block->label, "entry");
    assert(block->first_instr == NULL);
    assert(block->last_instr == NULL);

    free_mir_block(block);
    printf("✓ Block creation test passed\n");
}

void test_instruction_creation(void)
{
    printf("\nTesting instruction creation...\n");

    Type *i32_type = create_type(TYPE_I32);

    // Test ADD instruction
    MIROperand operands[3];
    operands[0] = create_reg_operand(1, i32_type);  // dest
    operands[1] = create_reg_operand(2, i32_type);  // src1
    operands[2] = create_imm_operand(42, i32_type); // src2

    MIRInstr *add_instr = create_mir_instr(MIR_ADD, operands, 3);
    assert(add_instr != NULL);
    assert(add_instr->op == MIR_ADD);
    assert(add_instr->operand_count == 3);
    assert(add_instr->operands[0].kind == MIR_REG);
    assert(add_instr->operands[1].kind == MIR_REG);
    assert(add_instr->operands[2].kind == MIR_IMM);

    // Test validation
    assert(validate_mir_instr(add_instr));

    free_mir_instr(add_instr);
    free_type(i32_type);
    printf("✓ Instruction creation test passed\n");
}

void test_block_instruction_addition(void)
{
    printf("\nTesting block instruction addition...\n");

    MIRBlock *block = create_mir_block("test");
    Type *i32_type = create_type(TYPE_I32);

    // Create and add multiple instructions
    MIROperand add_ops[3] = {
        create_reg_operand(1, i32_type),
        create_reg_operand(2, i32_type),
        create_reg_operand(3, i32_type)};
    MIRInstr *add = create_mir_instr(MIR_ADD, add_ops, 3);

    MIROperand ret_ops[1] = {
        create_reg_operand(1, i32_type)};
    MIRInstr *ret = create_mir_instr(MIR_RET, ret_ops, 1);

    mir_block_add_instr(block, add);
    mir_block_add_instr(block, ret);

    // Verify instructions were added correctly
    assert(block->first_instr == add);
    assert(block->last_instr == ret);
    assert(add->next == ret);
    assert(ret->next == NULL);

    free_mir_block(block);
    free_type(i32_type);
    printf("✓ Block instruction addition test passed\n");
}

void test_function_block_addition(void)
{
    printf("\nTesting function block addition...\n");

    Type *i32_type = create_type(TYPE_I32);
    MIRFunction *func = create_mir_function("test_func", i32_type);

    // Create and add multiple blocks
    MIRBlock *entry = create_mir_block("entry");
    MIRBlock *body = create_mir_block("body");
    MIRBlock *exit = create_mir_block("exit");

    mir_function_add_block(func, entry);
    mir_function_add_block(func, body);
    mir_function_add_block(func, exit);

    // Verify blocks were added correctly
    assert(func->first_block == entry);
    assert(func->last_block == exit);
    assert(entry->next == body);
    assert(body->next == exit);
    assert(exit->next == NULL);

    free_mir_function(func);
    free_type(i32_type);
    printf("✓ Function block addition test passed\n");
}

void test_validation(void)
{
    printf("\nTesting validation...\n");

    // Create a simple valid module
    MIRModule *module = create_mir_module();
    Type *i32_type = create_type(TYPE_I32);

    // Create function
    MIRFunction *func = create_mir_function("main", i32_type);
    mir_function_add_param(func, "x", i32_type);

    // Create block
    MIRBlock *block = create_mir_block("entry");

    // Create instructions
    MIROperand add_ops[3] = {
        create_reg_operand(1, i32_type),
        create_reg_operand(0, i32_type),
        create_imm_operand(42, i32_type)};
    MIRInstr *add = create_mir_instr(MIR_ADD, add_ops, 3);

    MIROperand ret_ops[1] = {
        create_reg_operand(1, i32_type)};
    MIRInstr *ret = create_mir_instr(MIR_RET, ret_ops, 1);

    // Build the function
    mir_block_add_instr(block, add);
    mir_block_add_instr(block, ret);
    mir_function_add_block(func, block);
    mir_module_add_function(module, func);

    // Test validation at all levels
    assert(validate_mir_instr(add));
    assert(validate_mir_instr(ret));
    assert(validate_mir_block(block));
    assert(validate_mir_function(func));
    assert(validate_mir_module(module));

    free_mir_module(module);
    free_type(i32_type);
    printf("✓ Validation test passed\n");
}

int main(void)
{
    printf("Running MIR system tests...\n");

    test_module_creation();
    test_function_creation();
    test_block_creation();
    test_instruction_creation();
    test_block_instruction_addition();
    test_function_block_addition();
    test_validation();

    printf("\nAll MIR system tests passed! ✓\n");
    return 0;
}