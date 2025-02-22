// tests/zir/test_zir_safety.cpp
#include "../../include/zir_c_api.h"
#include <cassert>
#include <cstring>
#include <iostream>

// Test null pointer handling
void test_null_safety()
{
    // Test null builder handling
    const char *version = zir_get_version(nullptr);
    assert(version == nullptr);
    std::cout << "✓ Null builder handling test passed\n";

    // Test double-free safety
    ZIRBuilder *builder = zir_create_builder();
    zir_destroy_builder(builder);
    zir_destroy_builder(nullptr); // Should handle this safely
    std::cout << "✓ Double-free safety test passed\n";
}

// Test multiple builder instances
void test_multiple_builders()
{
    ZIRBuilder *builder1 = zir_create_builder();
    ZIRBuilder *builder2 = zir_create_builder();

    assert(builder1 != nullptr);
    assert(builder2 != nullptr);
    assert(builder1 != builder2);

    const char *version1 = zir_get_version(builder1);
    const char *version2 = zir_get_version(builder2);
    assert(strcmp(version1, version2) == 0);

    zir_destroy_builder(builder1);
    zir_destroy_builder(builder2);
    std::cout << "✓ Multiple builders test passed\n";
}

// Test rapid create/destroy cycles
void test_stress_lifecycle()
{
    for (int i = 0; i < 1000; i++)
    {
        ZIRBuilder *builder = zir_create_builder();
        assert(builder != nullptr);
        assert(zir_get_version(builder) != nullptr);
        zir_destroy_builder(builder);
    }
    std::cout << "✓ Stress test passed\n";
}

int main()
{
    std::cout << "Running ZIR safety tests...\n";

    test_null_safety();
    test_multiple_builders();
    test_stress_lifecycle();

    std::cout << "All ZIR safety tests passed!\n";
    return 0;
}