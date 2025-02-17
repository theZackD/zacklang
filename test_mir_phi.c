#include "include/mir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test phi node creation
void test_phi_creation(void)
{
    printf("\nTesting MIR phi node creation...\n");

    // Create a type for our test
    Type *i32_type = create_type(TYPE_I32);

    // Create result register
    MIROperand result = create_reg_operand(0, i32_type);

    // Create incoming values and blocks
    MIROperand val1 = create_reg_operand(1, i32_type);
    MIROperand val2 = create_reg_operand(2, i32_type);
    MIROperand block1 = create_label_operand("block1");
    MIROperand block2 = create_label_operand("block2");

    MIROperand values[] = {val1, val2};
    MIROperand blocks[] = {block1, block2};

    // Create phi instruction
    MIRInstr *phi = create_mir_phi(result, values, blocks, 2);

    // Verify phi instruction
    assert(phi != NULL);
    assert(phi->op == MIR_PHI);
    assert(phi->operand_count == 5); // result + 2*(value + block)

    // Check result operand
    assert(phi->operands[0].kind == MIR_REG);
    assert(phi->operands[0].type == i32_type);
    assert(phi->operands[0].value.reg_num == 0);

    // Check first incoming value and block
    assert(phi->operands[1].kind == MIR_REG);
    assert(phi->operands[1].value.reg_num == 1);
    assert(phi->operands[2].kind == MIR_LABEL);
    assert(strcmp(phi->operands[2].value.label, "block1") == 0);

    // Check second incoming value and block
    assert(phi->operands[3].kind == MIR_REG);
    assert(phi->operands[3].value.reg_num == 2);
    assert(phi->operands[4].kind == MIR_LABEL);
    assert(strcmp(phi->operands[4].value.label, "block2") == 0);

    // Clean up
    free_mir_instr(phi);
    free_type(i32_type);

    printf("✓ MIR phi node creation test passed\n");
}

// Test adding incoming values to phi nodes
void test_phi_add_incoming(void)
{
    printf("\nTesting MIR phi node incoming value addition...\n");

    // Create a type for our test
    Type *i32_type = create_type(TYPE_I32);

    // Create result register
    MIROperand result = create_reg_operand(0, i32_type);

    // Create an empty phi instruction
    MIRInstr *phi = create_mir_phi(result, NULL, NULL, 0);

    // Add incoming values one at a time
    MIROperand val1 = create_reg_operand(1, i32_type);
    MIROperand block1 = create_label_operand("block1");
    mir_phi_add_incoming(phi, val1, block1);

    assert(phi->operand_count == 3); // result + value + block
    assert(phi->operands[1].kind == MIR_REG);
    assert(phi->operands[1].value.reg_num == 1);
    assert(phi->operands[2].kind == MIR_LABEL);
    assert(strcmp(phi->operands[2].value.label, "block1") == 0);

    // Add another incoming value
    MIROperand val2 = create_reg_operand(2, i32_type);
    MIROperand block2 = create_label_operand("block2");
    mir_phi_add_incoming(phi, val2, block2);

    assert(phi->operand_count == 5); // result + 2*(value + block)
    assert(phi->operands[3].kind == MIR_REG);
    assert(phi->operands[3].value.reg_num == 2);
    assert(phi->operands[4].kind == MIR_LABEL);
    assert(strcmp(phi->operands[4].value.label, "block2") == 0);

    // Clean up
    free_mir_instr(phi);
    free_type(i32_type);

    printf("✓ MIR phi node incoming value addition test passed\n");
}

// Test phi node validation
void test_phi_validation(void)
{
    printf("\nTesting MIR phi node validation...\n");

    Type *i32_type = create_type(TYPE_I32);
    Type *f64_type = create_type(TYPE_F64);

    // Test 1: Valid phi node
    MIROperand result1 = create_reg_operand(0, i32_type);
    MIROperand val1 = create_reg_operand(1, i32_type);
    MIROperand val2 = create_reg_operand(2, i32_type);
    MIROperand block1 = create_label_operand("block1");
    MIROperand block2 = create_label_operand("block2");

    MIROperand values1[] = {val1, val2};
    MIROperand blocks1[] = {block1, block2};

    MIRInstr *phi1 = create_mir_phi(result1, values1, blocks1, 2);
    assert(validate_mir_instr(phi1));

    // Test 2: Phi node with mismatched types
    MIROperand result2 = create_reg_operand(3, i32_type);
    MIROperand val3 = create_reg_operand(4, f64_type); // Different type

    MIROperand values2[] = {val1, val3};
    MIROperand blocks2[] = {block1, block2};

    MIRInstr *phi2 = create_mir_phi(result2, values2, blocks2, 2);
    assert(!validate_mir_instr(phi2));

    // Test 3: Empty phi node
    MIROperand result3 = create_reg_operand(5, i32_type);
    MIRInstr *phi3 = create_mir_phi(result3, NULL, NULL, 0);
    assert(validate_mir_instr(phi3));

    // Clean up
    free_mir_instr(phi1);
    free_mir_instr(phi2);
    free_mir_instr(phi3);

    // Note: Don't free individual operands as they are copied into the instruction
    free_type(i32_type);
    free_type(f64_type);

    printf("✓ MIR phi node validation test passed\n");
}

int main(void)
{
    printf("Running MIR phi node tests...\n");

    test_phi_creation();
    test_phi_add_incoming();
    test_phi_validation();

    printf("\nAll MIR phi node tests passed! ✓\n");
    return 0;
}