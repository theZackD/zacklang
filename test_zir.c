#include "include/zir.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

// Test helper function
static void assert_str_eq(const char *actual, const char *expected)
{
    if (strcmp(actual, expected) != 0)
    {
        printf("Expected '%s' but got '%s'\n", expected, actual);
        assert(0);
    }
}

void test_module_creation()
{
    printf("\nTesting module creation...\n");

    ZIRModule *module = create_zir_module();
    assert(module != NULL);
    assert(module->func_count == 0);
    assert(module->capacity > 0);

    free_zir_module(module);
    printf("✓ Module creation tests passed\n");
}

void test_function_creation()
{
    printf("\nTesting function creation...\n");

    Type *return_type = create_type(TYPE_I32);
    ZIRFunction *func = create_zir_function("main", return_type, false);

    assert(func != NULL);
    assert_str_eq(func->name, "main");
    assert(func->return_type == return_type);
    assert(!func->is_comptime);
    assert(func->param_count == 0);

    // Add a parameter
    Type *param_type = create_type(TYPE_F64);
    zir_function_add_param(func, "x", param_type);
    assert(func->param_count == 1);
    assert_str_eq(func->params[0].name, "x");
    assert(func->params[0].type == param_type);

    free_zir_function(func);
    free_type(return_type);
    free_type(param_type);
    printf("✓ Function creation tests passed\n");
}

void test_block_creation()
{
    printf("\nTesting block creation...\n");

    ZIRBlock *block = create_zir_block("entry");
    assert(block != NULL);
    assert_str_eq(block->label, "entry");
    assert(block->instr_count == 0);
    assert(block->capacity > 0);

    free_zir_block(block);
    printf("✓ Block creation tests passed\n");
}

void test_value_creation()
{
    printf("\nTesting value creation...\n");

    // Test constant creation
    Type *i32_type = create_type(TYPE_I32);
    ZIRValue *const_val = create_zir_const(i32_type, "42");
    assert(const_val != NULL);
    assert(const_val->kind == ZIR_CONST);
    assert(const_val->type == i32_type);
    assert(const_val->is_comptime);
    assert_str_eq(const_val->data.constant.literal_value, "42");

    // Test local variable creation
    ZIRValue *local_val = create_zir_local(i32_type, "x", true);
    assert(local_val != NULL);
    assert(local_val->kind == ZIR_LOCAL);
    assert(local_val->type == i32_type);
    assert(!local_val->is_comptime);
    assert_str_eq(local_val->data.local.name, "x");
    assert(local_val->data.local.is_mutable);

    // Test load operation
    ZIRValue *load_val = create_zir_load(local_val);
    assert(load_val != NULL);
    assert(load_val->kind == ZIR_LOAD);
    assert(load_val->type == i32_type);
    assert(load_val->data.load.address == local_val);

    // Test store operation
    ZIRValue *store_val = create_zir_store(local_val, const_val);
    assert(store_val != NULL);
    assert(store_val->kind == ZIR_STORE);
    assert(store_val->data.store.address == local_val);
    assert(store_val->data.store.value == const_val);

    // Test binary operation
    ZIRValue *binary_val = create_zir_binary("+", load_val, const_val);
    assert(binary_val != NULL);
    assert(binary_val->kind == ZIR_BINARY);
    assert_str_eq(binary_val->data.binary.op, "+");
    assert(binary_val->data.binary.left == load_val);
    assert(binary_val->data.binary.right == const_val);

    free_zir_value(binary_val);
    free_zir_value(store_val);
    free_zir_value(load_val);
    free_zir_value(local_val);
    free_zir_value(const_val);
    free_type(i32_type);
    printf("✓ Value creation tests passed\n");
}

void test_comptime_propagation()
{
    printf("\nTesting comptime propagation...\n");

    Type *i32_type = create_type(TYPE_I32);

    // Create two comptime constants
    ZIRValue *const1 = create_zir_const(i32_type, "40");
    ZIRValue *const2 = create_zir_const(i32_type, "2");
    assert(const1->is_comptime && const2->is_comptime);

    // Binary operation with two comptime values should be comptime
    ZIRValue *add = create_zir_binary("+", const1, const2);
    assert(add->is_comptime);

    // Create a runtime value
    ZIRValue *local = create_zir_local(i32_type, "x", true);
    assert(!local->is_comptime);

    // Operation mixing runtime and comptime should be runtime
    ZIRValue *mixed = create_zir_binary("+", add, local);
    assert(!mixed->is_comptime);

    free_zir_value(mixed);
    free_zir_value(add);
    free_zir_value(local);
    free_zir_value(const2);
    free_zir_value(const1);
    free_type(i32_type);
    printf("✓ Comptime propagation tests passed\n");
}

void test_validation()
{
    printf("\nTesting validation...\n");

    // Create a simple valid module
    ZIRModule *module = create_zir_module();
    Type *i32_type = create_type(TYPE_I32);

    ZIRFunction *func = create_zir_function("main", i32_type, false);
    ZIRBlock *block = create_zir_block("entry");
    ZIRValue *const_val = create_zir_const(i32_type, "42");
    ZIRValue *ret = create_zir_return(const_val);

    zir_block_add_instr(block, const_val);
    zir_block_add_instr(block, ret);
    zir_function_add_block(func, block);
    zir_module_add_function(module, func);

    // Test validation
    assert(validate_zir_value(const_val));
    assert(validate_zir_value(ret));
    assert(validate_zir_block(block));
    assert(validate_zir_function(func));
    assert(validate_zir_module(module));

    free_zir_module(module);
    free_type(i32_type);
    printf("✓ Validation tests passed\n");
}

int main()
{
    printf("Running ZIR system tests...\n");

    test_module_creation();
    test_function_creation();
    test_block_creation();
    test_value_creation();
    test_comptime_propagation();
    test_validation();

    printf("\nAll ZIR system tests passed! ✓\n");
    return 0;
}