#include "../../include/zir_c_api.h"
#include <cassert>
#include <cstring>
#include <iostream>
#include <cstdio>

void test_basic_block_creation()
{
    // Test creating a block
    ZIRBasicBlockHandle block = zir_create_basic_block("test");
    assert(block != nullptr);
    assert(strcmp(zir_get_block_name(block), "test") == 0);

    // Test ID functionality
    uint64_t id = zir_get_block_id(block);
    assert(id != UINT64_MAX); // Valid ID

    // Test changing block name
    zir_set_block_name(block, "modified");
    assert(strcmp(zir_get_block_name(block), "modified") == 0);

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
    // Create multiple blocks
    ZIRBasicBlockHandle blocks[3];
    const char *names[] = {"entry", "body", "exit"};

    for (int i = 0; i < 3; i++)
    {
        blocks[i] = zir_create_basic_block(names[i]);
        assert(blocks[i] != nullptr);
        assert(strcmp(zir_get_block_name(blocks[i]), names[i]) == 0);
    }

    // Verify they're distinct
    for (int i = 0; i < 3; i++)
    {
        for (int j = i + 1; j < 3; j++)
        {
            assert(blocks[i] != blocks[j]);
        }
    }

    // Test unique IDs
    uint64_t ids[3];
    for (int i = 0; i < 3; i++)
    {
        ids[i] = zir_get_block_id(blocks[i]);
        assert(ids[i] != UINT64_MAX);
    }

    // Verify IDs are unique
    for (int i = 0; i < 3; i++)
    {
        for (int j = i + 1; j < 3; j++)
        {
            assert(ids[i] != ids[j]);
        }
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