#include "zir_c_api.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <vector>

void test_basic_block_creation()
{
    std::cout << "Testing basic block creation...\n";

    // Create a basic block
    zir_block_handle block = zir_create_basic_block("test");
    assert(block != nullptr);

    // Get and verify the name
    const char *name = zir_get_block_name(block);
    assert(name != nullptr);
    assert(strcmp(name, "test") == 0);

    // Get and verify the ID
    uint64_t id = zir_get_block_id(block);
    assert(id != UINT64_MAX);

    // Cleanup
    zir_destroy_basic_block(block);
    std::cout << "✓ Basic block creation test passed\n";
}

void test_null_handling()
{
    // Test null name in creation
    assert(zir_create_basic_block(nullptr) == nullptr);

    // Test null block in operations
    assert(zir_get_block_name(nullptr) == nullptr);
    assert(zir_get_block_id(nullptr) == UINT64_MAX);

    // These should not crash
    zir_set_block_name(nullptr, "test");
    zir_destroy_basic_block(nullptr);

    std::cout << "✓ Null handling test passed\n";
}

void test_multiple_blocks()
{
    std::cout << "Testing multiple blocks...\n";

    const size_t num_blocks = 10;
    std::vector<zir_block_handle> blocks(num_blocks);
    const char *names[] = {
        "block0", "block1", "block2", "block3", "block4",
        "block5", "block6", "block7", "block8", "block9"};

    // Create blocks
    for (size_t i = 0; i < num_blocks; i++)
    {
        blocks[i] = zir_create_basic_block(names[i]);
        assert(blocks[i] != nullptr);

        // Verify name
        const char *name = zir_get_block_name(blocks[i]);
        assert(name != nullptr);
        assert(strcmp(name, names[i]) == 0);

        // Verify ID
        uint64_t id = zir_get_block_id(blocks[i]);
        assert(id != UINT64_MAX);
    }

    // Cleanup
    for (auto block : blocks)
    {
        zir_destroy_basic_block(block);
    }

    std::cout << "✓ Multiple blocks test passed\n";
}

int main()
{
    std::cout << "Running ZIR basic block tests...\n";

    test_basic_block_creation();
    test_null_handling();
    test_multiple_blocks();

    std::cout << "All ZIR basic block tests passed!\n";
    return 0;
}