// tests/zir/test_zir_basic.cpp
#include "../../include/zir_c_api.h"
#include <cassert>
#include <cstring>
#include <iostream>

void test_builder_lifecycle()
{
    // Create builder
    ZIRBuilder *builder = zir_create_builder();
    assert(builder != nullptr);
    std::cout << "✓ Builder creation test passed\n";

    // Get version
    const char *version = zir_get_version(builder);
    assert(version != nullptr);
    assert(strcmp(version, "0.1.0") == 0);
    std::cout << "✓ Version check test passed\n";

    // Destroy builder
    zir_destroy_builder(builder);
    std::cout << "✓ Builder destruction test passed\n";
}

int main()
{
    std::cout << "Running basic ZIR tests...\n";
    test_builder_lifecycle();
    std::cout << "All basic ZIR tests passed!\n";
    return 0;
}