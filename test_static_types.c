#include "include/static_types.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

void test_type_creation()
{
    printf("\nTesting type creation...\n");

    Type *t1 = create_type(TYPE_I32);
    assert(t1->kind == TYPE_I32);
    assert(!t1->is_const);
    assert(!t1->is_comptime);

    Type *t2 = create_const_type(TYPE_F64);
    assert(t2->kind == TYPE_F64);
    assert(t2->is_const);
    assert(!t2->is_comptime);

    Type *t3 = create_comptime_type(TYPE_I32);
    assert(t3->kind == TYPE_I32);
    assert(!t3->is_const);
    assert(t3->is_comptime);

    // Test type string representation
    assert(strcmp(type_to_string(t1), "i32") == 0);
    assert(strcmp(type_to_string(t2), "const f64") == 0);
    assert(strcmp(type_to_string(t3), "comptime i32") == 0);

    free_type(t1);
    free_type(t2);
    free_type(t3);
    printf("✓ Type creation tests passed\n");
}

void test_type_safety()
{
    printf("\nTesting type safety...\n");

    Type *i32_type = create_type(TYPE_I32);
    Type *i64_type = create_type(TYPE_I64);
    Type *const_i32 = create_const_type(TYPE_I32);
    Type *comptime_i32 = create_comptime_type(TYPE_I32);

    // Test exact equality
    assert(types_are_equal(i32_type, i32_type));
    assert(!types_are_equal(i32_type, i64_type));
    assert(!types_are_equal(i32_type, const_i32));
    assert(!types_are_equal(i32_type, comptime_i32));

    // Test type safety rules
    assert(!type_is_safe_for(i32_type, i64_type));     // No implicit conversion
    assert(!type_is_safe_for(i64_type, i32_type));     // No implicit conversion
    assert(!type_is_safe_for(i32_type, const_i32));    // Can't assign to const
    assert(type_is_safe_for(const_i32, i32_type));     // Can use const where non-const expected
    assert(!type_is_safe_for(i32_type, comptime_i32)); // Runtime value not safe for comptime

    free_type(i32_type);
    free_type(i64_type);
    free_type(const_i32);
    free_type(comptime_i32);
    printf("✓ Type safety tests passed\n");
}

void test_numeric_operations()
{
    printf("\nTesting numeric operations...\n");

    Type *i32_type = create_type(TYPE_I32);
    Type *i64_type = create_type(TYPE_I64);
    Type *f64_type = create_type(TYPE_F64);

    // Test numeric type checks
    assert(is_numeric_type(i32_type));
    assert(is_numeric_type(f64_type));
    assert(is_integer_type(i32_type));
    assert(!is_integer_type(f64_type));
    assert(is_float_type(f64_type));
    assert(!is_float_type(i32_type));

    // Test value range checks
    assert(value_fits_in_type("42", i32_type));
    assert(value_fits_in_type("-42", i32_type));
    assert(!value_fits_in_type("9999999999999999999", i32_type)); // Too big for i32
    assert(value_fits_in_type("9999999999999999999", i64_type));  // Fits in i64
    assert(value_fits_in_type("3.14", f64_type));
    assert(!value_fits_in_type("not_a_number", i32_type));

    free_type(i32_type);
    free_type(i64_type);
    free_type(f64_type);
    printf("✓ Numeric operation tests passed\n");
}

void test_comptime_evaluation()
{
    printf("\nTesting comptime evaluation...\n");

    // Test literal type inference (all literals should be comptime)
    Type *int_lit = get_literal_type("42");
    assert(int_lit->is_comptime);
    assert(int_lit->kind == TYPE_I32);

    Type *float_lit = get_literal_type("3.14");
    assert(float_lit->is_comptime);
    assert(float_lit->kind == TYPE_F64);

    Type *bool_lit = get_literal_type("true");
    assert(bool_lit->is_comptime);
    assert(bool_lit->kind == TYPE_BOOL);

    // Test comptime operation results
    Type *comptime_i32 = create_comptime_type(TYPE_I32);
    Type *runtime_i32 = create_type(TYPE_I32);

    Type *result1 = get_binary_op_type("+", comptime_i32, comptime_i32);
    assert(result1->is_comptime); // Operation on two comptime values is comptime

    Type *result2 = get_binary_op_type("+", comptime_i32, runtime_i32);
    assert(!result2->is_comptime); // Operation with runtime value is runtime

    free_type(int_lit);
    free_type(float_lit);
    free_type(bool_lit);
    free_type(comptime_i32);
    free_type(runtime_i32);
    free_type(result1);
    free_type(result2);
    printf("✓ Comptime evaluation tests passed\n");
}

void test_type_conversion()
{
    printf("\nTesting type conversion...\n");

    Type *i32_type = create_type(TYPE_I32);
    Type *i64_type = create_type(TYPE_I64);
    Type *f64_type = create_type(TYPE_F64);

    // Test conversion function names
    const char *conv1 = get_type_conversion_fn(i32_type, i64_type);
    assert(strcmp(conv1, "as_i64") == 0);

    const char *conv2 = get_type_conversion_fn(i32_type, f64_type);
    assert(strcmp(conv2, "as_f64") == 0);

    // No conversion needed for same type
    const char *conv3 = get_type_conversion_fn(i32_type, i32_type);
    assert(conv3 == NULL);

    free_type(i32_type);
    free_type(i64_type);
    free_type(f64_type);
    printf("✓ Type conversion tests passed\n");
}

int main()
{
    printf("Running static type system tests...\n");

    test_type_creation();
    test_type_safety();
    test_numeric_operations();
    test_comptime_evaluation();
    test_type_conversion();

    printf("\nAll static type system tests passed! ✓\n");
    return 0;
}