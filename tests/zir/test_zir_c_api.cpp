#include "../../include/zir_c_api.h"
#include <cassert>
#include <iostream>
#include <cstring>

void test_integer_types()
{
    // Test i32 type
    ZIRTypeHandle i32_type = zir_create_i32_type();
    assert(i32_type != nullptr);
    assert(zir_is_integer_type(i32_type));
    assert(!zir_is_float_type(i32_type));
    assert(!zir_is_bool_type(i32_type));
    assert(!zir_is_string_type(i32_type));

    // Test i64 type
    ZIRTypeHandle i64_type = zir_create_i64_type();
    assert(i64_type != nullptr);
    assert(zir_is_integer_type(i64_type));

    // Test integer literals
    ZIRValueHandle i32_val = zir_create_integer_literal(i32_type, 42);
    assert(i32_val != nullptr);
    assert(zir_get_integer_value(i32_val) == 42);

    ZIRValueHandle i64_val = zir_create_integer_literal(i64_type, 9223372036854775807LL);
    assert(i64_val != nullptr);
    assert(zir_get_integer_value(i64_val) == 9223372036854775807LL);

    // Cleanup
    zir_destroy_value(i32_val);
    zir_destroy_value(i64_val);
    zir_destroy_type(i32_type);
    zir_destroy_type(i64_type);

    std::cout << "✓ Integer C API tests passed\n";
}

void test_float_types()
{
    // Test f32 type
    ZIRTypeHandle f32_type = zir_create_f32_type();
    assert(f32_type != nullptr);
    assert(zir_is_float_type(f32_type));
    assert(!zir_is_integer_type(f32_type));

    // Test f64 type
    ZIRTypeHandle f64_type = zir_create_f64_type();
    assert(f64_type != nullptr);
    assert(zir_is_float_type(f64_type));

    // Test float literals
    ZIRValueHandle f32_val = zir_create_float_literal(f32_type, 3.14159f);
    assert(f32_val != nullptr);
    assert(abs(zir_get_float_value(f32_val) - 3.14159) < 0.0001);

    ZIRValueHandle f64_val = zir_create_float_literal(f64_type, 3.14159265359);
    assert(f64_val != nullptr);
    assert(abs(zir_get_float_value(f64_val) - 3.14159265359) < 0.0000000001);

    // Cleanup
    zir_destroy_value(f32_val);
    zir_destroy_value(f64_val);
    zir_destroy_type(f32_type);
    zir_destroy_type(f64_type);

    std::cout << "✓ Float C API tests passed\n";
}

void test_bool_type()
{
    // Test bool type
    ZIRTypeHandle bool_type = zir_create_bool_type();
    assert(bool_type != nullptr);
    assert(zir_is_bool_type(bool_type));
    assert(!zir_is_integer_type(bool_type));

    // Test bool literals
    ZIRValueHandle true_val = zir_create_bool_literal(bool_type, true);
    assert(true_val != nullptr);
    assert(zir_get_bool_value(true_val) == true);

    ZIRValueHandle false_val = zir_create_bool_literal(bool_type, false);
    assert(false_val != nullptr);
    assert(zir_get_bool_value(false_val) == false);

    // Cleanup
    zir_destroy_value(true_val);
    zir_destroy_value(false_val);
    zir_destroy_type(bool_type);

    std::cout << "✓ Boolean C API tests passed\n";
}

void test_string_type()
{
    // Test string type
    ZIRTypeHandle string_type = zir_create_string_type();
    assert(string_type != nullptr);
    assert(zir_is_string_type(string_type));
    assert(!zir_is_integer_type(string_type));

    // Test string literals
    ZIRValueHandle str_val = zir_create_string_literal(string_type, "Hello, World!");
    assert(str_val != nullptr);
    assert(strcmp(zir_get_string_value(str_val), "Hello, World!") == 0);

    ZIRValueHandle empty_val = zir_create_string_literal(string_type, "");
    assert(empty_val != nullptr);
    assert(strcmp(zir_get_string_value(empty_val), "") == 0);

    // Cleanup
    zir_destroy_value(str_val);
    zir_destroy_value(empty_val);
    zir_destroy_type(string_type);

    std::cout << "✓ String C API tests passed\n";
}

void test_error_cases()
{
    // Test null type handle
    assert(zir_create_integer_literal(nullptr, 42) == nullptr);
    assert(zir_create_float_literal(nullptr, 3.14) == nullptr);
    assert(zir_create_bool_literal(nullptr, true) == nullptr);
    assert(zir_create_string_literal(nullptr, "test") == nullptr);

    // Test type checking with null handle
    assert(!zir_is_integer_type(nullptr));
    assert(!zir_is_float_type(nullptr));
    assert(!zir_is_bool_type(nullptr));
    assert(!zir_is_string_type(nullptr));

    // Test value access with null handle
    assert(zir_get_integer_value(nullptr) == 0);
    assert(zir_get_float_value(nullptr) == 0.0);
    assert(zir_get_bool_value(nullptr) == false);
    assert(zir_get_string_value(nullptr) == nullptr);

    // Test cleanup with null handle (should not crash)
    zir_destroy_type(nullptr);
    zir_destroy_value(nullptr);

    std::cout << "✓ Error case C API tests passed\n";
}

int main()
{
    std::cout << "Running ZIR C API tests...\n";

    test_integer_types();
    test_float_types();
    test_bool_type();
    test_string_type();
    test_error_cases();

    std::cout << "All ZIR C API tests passed!\n";
    return 0;
}