#include "include/zir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test phi node creation
void test_phi_creation(void)
{
    printf("\nTesting phi node creation...\n");

    // Create a type for our test
    Type *i32_type = create_type(TYPE_I32);

    // Create some blocks
    ZIRBlock *block1 = create_zir_block("block1");
    ZIRBlock *block2 = create_zir_block("block2");

    // Create some values
    ZIRValue *val1 = create_zir_const(i32_type, "1");
    ZIRValue *val2 = create_zir_const(i32_type, "2");

    // Create arrays for incoming values and blocks
    ZIRValue *values[] = {val1, val2};
    ZIRBlock *blocks[] = {block1, block2};

    // Create phi node
    ZIRValue *phi = create_zir_phi(i32_type, values, blocks, 2);

    // Verify phi node
    assert(phi != NULL);
    assert(phi->kind == ZIR_PHI);
    assert(phi->type == i32_type);
    assert(phi->data.phi.incoming_count == 2);
    assert(phi->data.phi.incoming_values[0] == val1);
    assert(phi->data.phi.incoming_values[1] == val2);
    assert(phi->data.phi.incoming_blocks[0] == block1);
    assert(phi->data.phi.incoming_blocks[1] == block2);

    // Test comptime propagation (both values are comptime)
    assert(phi->is_comptime == true);

    // Clean up
    free_zir_value(phi);
    free_zir_value(val1);
    free_zir_value(val2);
    free_zir_block(block1);
    free_zir_block(block2);
    free_type(i32_type);

    printf("✓ Phi node creation test passed\n");
}

// Test adding incoming values to phi nodes
void test_phi_add_incoming(void)
{
    printf("\nTesting phi node incoming value addition...\n");

    // Create a type for our test
    Type *i32_type = create_type(TYPE_I32);

    // Create an empty phi node
    ZIRValue *phi = create_zir_phi(i32_type, NULL, NULL, 0);

    // Create some blocks and values
    ZIRBlock *block1 = create_zir_block("block1");
    ZIRBlock *block2 = create_zir_block("block2");
    ZIRValue *val1 = create_zir_const(i32_type, "1");
    ZIRValue *val2 = create_zir_const(i32_type, "2");

    // Add incoming values one at a time
    zir_phi_add_incoming(phi, val1, block1);
    assert(phi->data.phi.incoming_count == 1);
    assert(phi->data.phi.incoming_values[0] == val1);
    assert(phi->data.phi.incoming_blocks[0] == block1);

    zir_phi_add_incoming(phi, val2, block2);
    assert(phi->data.phi.incoming_count == 2);
    assert(phi->data.phi.incoming_values[1] == val2);
    assert(phi->data.phi.incoming_blocks[1] == block2);

    // Clean up
    free_zir_value(phi);
    free_zir_value(val1);
    free_zir_value(val2);
    free_zir_block(block1);
    free_zir_block(block2);
    free_type(i32_type);

    printf("✓ Phi node incoming value addition test passed\n");
}

// Test phi node validation
void test_phi_validation(void)
{
    printf("\nTesting phi node validation...\n");

    Type *i32_type = create_type(TYPE_I32);
    Type *f64_type = create_type(TYPE_F64);

    ZIRBlock *block1 = create_zir_block("block1");
    ZIRBlock *block2 = create_zir_block("block2");

    // Test 1: Valid phi node
    ZIRValue *val1 = create_zir_const(i32_type, "1");
    ZIRValue *val2 = create_zir_const(i32_type, "2");
    ZIRValue *values1[] = {val1, val2};
    ZIRBlock *blocks1[] = {block1, block2};
    ZIRValue *phi1 = create_zir_phi(i32_type, values1, blocks1, 2);
    assert(validate_zir_value(phi1));

    // Test 2: Phi node with mismatched types
    ZIRValue *val3 = create_zir_const(f64_type, "3.14");
    ZIRValue *values2[] = {val1, val3};
    ZIRBlock *blocks2[] = {block1, block2};
    ZIRValue *phi2 = create_zir_phi(i32_type, values2, blocks2, 2);
    assert(!validate_zir_value(phi2));

    // Test 3: Empty phi node
    ZIRValue *phi3 = create_zir_phi(i32_type, NULL, NULL, 0);
    assert(validate_zir_value(phi3));

    // Clean up
    free_zir_value(phi1);
    free_zir_value(phi2);
    free_zir_value(phi3);
    free_zir_value(val1);
    free_zir_value(val2);
    free_zir_value(val3);
    free_zir_block(block1);
    free_zir_block(block2);
    free_type(i32_type);
    free_type(f64_type);

    printf("✓ Phi node validation test passed\n");
}

int main(void)
{
    printf("Running ZIR phi node tests...\n");

    test_phi_creation();
    test_phi_add_incoming();
    test_phi_validation();

    printf("\nAll ZIR phi node tests passed! ✓\n");
    return 0;
}