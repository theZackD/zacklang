#include "../../include/zir_c_api.h"
#include <cassert>
#include <cstring>
#include <iostream>

void test_function_creation()
{
    // Test creating a function
    ZIRFunctionHandle function = zir_create_function("main");
    assert(function != nullptr);
    assert(strcmp(zir_get_function_name(function), "main") == 0);

    // Test changing function name
    zir_set_function_name(function, "test_func");
    assert(strcmp(zir_get_function_name(function), "test_func") == 0);

    // Cleanup
    zir_destroy_function(function);
    std::cout << "✓ Function creation test passed\n";
}

void test_null_handling()
{
    // Test null name in creation
    assert(zir_create_function(nullptr) == nullptr);

    // Test null function in operations
    assert(zir_get_function_name(nullptr) == nullptr);
    assert(zir_function_get_block_count(nullptr) == 0);
    assert(zir_function_get_block(nullptr, 0) == nullptr);

    // These should not crash
    zir_set_function_name(nullptr, "test");
    zir_destroy_function(nullptr);
    zir_function_add_block(nullptr, nullptr);
    zir_function_remove_block(nullptr, nullptr);

    std::cout << "✓ Null handling test passed\n";
}

void test_block_management()
{
    // Create a function and some blocks
    ZIRFunctionHandle function = zir_create_function("test_func");
    ZIRBasicBlockHandle block1 = zir_create_basic_block("entry");
    ZIRBasicBlockHandle block2 = zir_create_basic_block("body");
    ZIRBasicBlockHandle block3 = zir_create_basic_block("exit");

    printf("Original function handle=%p\n", function);

    // Add blocks to function
    zir_function_add_block(function, block1);
    zir_function_add_block(function, block2);
    zir_function_add_block(function, block3);

    // Verify block count
    assert(zir_function_get_block_count(function) == 3);

    // Verify block order and parent relationships
    assert(zir_function_get_block(function, 0) != nullptr);
    assert(zir_function_get_block(function, 1) != nullptr);
    assert(zir_function_get_block(function, 2) != nullptr);
    assert(zir_function_get_block(function, 3) == nullptr);

    // Verify parent relationships
    printf("Parent handle=%p\n", zir_get_block_parent(block1));
    assert(zir_get_block_parent(block1) == function);
    assert(zir_get_block_parent(block2) == function);
    assert(zir_get_block_parent(block3) == function);

    // Test removing a block
    zir_function_remove_block(function, block2);
    assert(zir_function_get_block_count(function) == 2);
    assert(zir_get_block_parent(block2) == nullptr);

    // Cleanup
    zir_destroy_basic_block(block1);
    zir_destroy_basic_block(block2);
    zir_destroy_basic_block(block3);
    zir_destroy_function(function);
    std::cout << "✓ Block management test passed\n";
}

void test_multiple_functions()
{
    // Create multiple functions
    ZIRFunctionHandle func1 = zir_create_function("func1");
    ZIRFunctionHandle func2 = zir_create_function("func2");

    // Create blocks
    ZIRBasicBlockHandle block1 = zir_create_basic_block("block1");
    ZIRBasicBlockHandle block2 = zir_create_basic_block("block2");

    // Add blocks to different functions
    zir_function_add_block(func1, block1);
    zir_function_add_block(func2, block2);

    // Verify block counts
    assert(zir_function_get_block_count(func1) == 1);
    assert(zir_function_get_block_count(func2) == 1);

    // Verify parent relationships
    assert(zir_get_block_parent(block1) == func1);
    assert(zir_get_block_parent(block2) == func2);

    // Move block from func2 to func1
    zir_function_remove_block(func2, block2);
    zir_function_add_block(func1, block2);
    assert(zir_function_get_block_count(func1) == 2);
    assert(zir_function_get_block_count(func2) == 0);
    assert(zir_get_block_parent(block2) == func1);

    // Cleanup
    zir_destroy_basic_block(block1);
    zir_destroy_basic_block(block2);
    zir_destroy_function(func1);
    zir_destroy_function(func2);
    std::cout << "✓ Multiple functions test passed\n";
}

int main()
{
    std::cout << "Running ZIR function tests...\n";

    test_function_creation();
    test_null_handling();
    test_block_management();
    test_multiple_functions();

    std::cout << "All ZIR function tests passed!\n";
    return 0;
}