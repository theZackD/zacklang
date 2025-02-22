#include "../../../include/zir_c_api.h"
#include <cassert>
#include <iostream>
#include <vector>

// Test memory leaks in normal operation
void test_normal_usage()
{
    ZIRBuilder *builder = zir_create_builder();
    const char *version = zir_get_version(builder);
    assert(version != nullptr);
    zir_destroy_builder(builder);
    std::cout << "✓ Normal usage memory test complete\n";
}

// Test memory leaks with multiple instances
void test_multiple_instances()
{
    std::vector<ZIRBuilder *> builders;

    // Create multiple builders
    for (int i = 0; i < 100; i++)
    {
        builders.push_back(zir_create_builder());
    }

    // Use each builder
    for (auto builder : builders)
    {
        const char *version = zir_get_version(builder);
        assert(version != nullptr);
    }

    // Cleanup
    for (auto builder : builders)
    {
        zir_destroy_builder(builder);
    }

    std::cout << "✓ Multiple instances memory test complete\n";
}

// Test memory handling in error conditions
void test_error_conditions()
{
    // Test with nullptr
    zir_get_version(nullptr);
    zir_destroy_builder(nullptr);

    // Test partial initialization
    ZIRBuilder *builder = zir_create_builder();
    // Intentionally don't use the builder
    zir_destroy_builder(builder);

    std::cout << "✓ Error conditions memory test complete\n";
}

int main()
{
    std::cout << "Running ZIR memory tests...\n";

    test_normal_usage();
    test_multiple_instances();
    test_error_conditions();

    std::cout << "All ZIR memory tests completed!\n";
    return 0;
}
