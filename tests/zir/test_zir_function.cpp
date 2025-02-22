#include "zir_c_api.h"
#include <cassert>
#include <cstring>
#include <iostream>

void test_function_creation()
{
    std::cout << "Testing function creation...\n";

    // Create a function
    zir_function_handle function = zir_create_function("test_func");
    assert(function != nullptr);

    // Get and verify the name
    const char *name = zir_get_function_name(function);
    assert(name != nullptr);
    assert(strcmp(name, "test_func") == 0);

    // Test setting a new name
    zir_set_function_name(function, "modified_func");
    name = zir_get_function_name(function);
    assert(strcmp(name, "modified_func") == 0);

    // Cleanup
    zir_destroy_function(function);
    std::cout << "✓ Function creation test passed\n";
}

void test_null_handling()
{
    // Test null function operations
    assert(zir_get_function_name(nullptr) == nullptr);
    zir_set_function_name(nullptr, "test");
    zir_destroy_function(nullptr);
    zir_function_add_block(nullptr, nullptr);
    assert(zir_function_get_block_count(nullptr) == 0);
    assert(zir_function_get_block(nullptr, 0) == nullptr);
    std::cout << "✓ Null handling test passed\n";
}

void test_function_block_management()
{
    std::cout << "Testing function block management...\n";

    // Create a function
    zir_function_handle function = zir_create_function("test_func");
    assert(function != nullptr);

    // Create blocks
    zir_block_handle block1 = zir_create_basic_block("block1");
    zir_block_handle block2 = zir_create_basic_block("block2");
    zir_block_handle block3 = zir_create_basic_block("block3");
    assert(block1 != nullptr && block2 != nullptr && block3 != nullptr);

    // Add blocks to function
    assert(zir_function_add_block(function, block1));
    assert(zir_function_add_block(function, block2));
    assert(zir_function_add_block(function, block3));

    // Verify block count
    size_t count = zir_function_get_block_count(function);
    assert(count == 3);

    // Get blocks back and verify
    zir_block_handle retrieved1 = zir_function_get_block(function, 0);
    zir_block_handle retrieved2 = zir_function_get_block(function, 1);
    zir_block_handle retrieved3 = zir_function_get_block(function, 2);
    assert(retrieved1 != nullptr && retrieved2 != nullptr && retrieved3 != nullptr);

    // Verify block names
    assert(strcmp(zir_get_block_name(retrieved1), "block1") == 0);
    assert(strcmp(zir_get_block_name(retrieved2), "block2") == 0);
    assert(strcmp(zir_get_block_name(retrieved3), "block3") == 0);

    // Remove a block
    zir_function_remove_block(function, block2);
    count = zir_function_get_block_count(function);
    assert(count == 2);

    // Cleanup
    zir_destroy_function(function);
    zir_destroy_basic_block(block1);
    zir_destroy_basic_block(block2);
    zir_destroy_basic_block(block3);
    std::cout << "✓ Function block management test passed\n";
}

int main()
{
    std::cout << "Running ZIR function tests...\n";

    test_function_creation();
    test_null_handling();
    test_function_block_management();

    std::cout << "All ZIR function tests passed!\n";
    return 0;
}