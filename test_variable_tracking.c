#include "include/variable_tracking.h"
#include "include/static_types.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Mock ZIRValue for testing
static ZIRValue *create_mock_zir_value(int mock_id)
{
    ZIRValue *value = malloc(sizeof(ZIRValue));
    value->kind = ZIR_CONST;
    value->type = create_type(TYPE_I32);
    value->is_comptime = true;

    // Convert mock_id to string for literal value
    char str_value[32];
    snprintf(str_value, sizeof(str_value), "%d", mock_id);
    value->data.constant.literal_value = strdup(str_value);

    return value;
}

// Helper to free mock ZIRValue
static void free_mock_zir_value(ZIRValue *value)
{
    if (value)
    {
        free_type(value->type);
        if (value->kind == ZIR_CONST)
        {
            free(value->data.constant.literal_value);
        }
        free(value);
    }
}

// Test creation and destruction
void test_table_creation(void)
{
    printf("\nTesting table creation and destruction...\n");

    VariableTable *table = create_variable_table(10);
    assert(table != NULL);
    assert(table->bucket_count == 10);
    assert(table->total_variables == 0);

    destroy_variable_table(table);
    printf("✓ Table creation test passed\n");
}

// Test variable version creation and retrieval
void test_version_tracking(void)
{
    printf("\nTesting variable version tracking...\n");

    VariableTable *table = create_variable_table(10);

    // Create mock values
    ZIRValue *val1 = create_mock_zir_value(1);
    ZIRValue *val2 = create_mock_zir_value(2);
    ZIRValue *val3 = create_mock_zir_value(3);

    // Test initial state
    assert(get_current_version(table, "x") == -1);
    assert(!has_variable(table, "x"));

    // Test version creation
    int v1 = create_new_version(table, "x", val1);
    assert(v1 == 0);
    assert(has_variable(table, "x"));
    assert(get_current_version(table, "x") == 0);

    int v2 = create_new_version(table, "x", val2);
    assert(v2 == 1);
    assert(get_current_version(table, "x") == 1);

    int v3 = create_new_version(table, "x", val3);
    assert(v3 == 2);
    assert(get_current_version(table, "x") == 2);

    // Test version retrieval
    assert(get_version_definition(table, "x", 0) == val1);
    assert(get_version_definition(table, "x", 1) == val2);
    assert(get_version_definition(table, "x", 2) == val3);
    assert(get_current_definition(table, "x") == val3);

    // Clean up
    free_mock_zir_value(val1);
    free_mock_zir_value(val2);
    free_mock_zir_value(val3);
    destroy_variable_table(table);

    printf("✓ Version tracking test passed\n");
}

// Test multiple variables
void test_multiple_variables(void)
{
    printf("\nTesting multiple variables...\n");

    VariableTable *table = create_variable_table(10);

    // Create mock values
    ZIRValue *x1 = create_mock_zir_value(1);
    ZIRValue *x2 = create_mock_zir_value(2);
    ZIRValue *y1 = create_mock_zir_value(3);
    ZIRValue *y2 = create_mock_zir_value(4);

    // Create versions for x and y
    create_new_version(table, "x", x1);
    create_new_version(table, "y", y1);
    create_new_version(table, "x", x2);
    create_new_version(table, "y", y2);

    // Test version counts
    assert(get_version_count(table, "x") == 2);
    assert(get_version_count(table, "y") == 2);

    // Test current versions
    assert(get_current_version(table, "x") == 1);
    assert(get_current_version(table, "y") == 1);

    // Test definitions
    assert(get_current_definition(table, "x") == x2);
    assert(get_current_definition(table, "y") == y2);

    // Clean up
    free_mock_zir_value(x1);
    free_mock_zir_value(x2);
    free_mock_zir_value(y1);
    free_mock_zir_value(y2);
    destroy_variable_table(table);

    printf("✓ Multiple variables test passed\n");
}

// Test edge cases and error handling
void test_edge_cases(void)
{
    printf("\nTesting edge cases...\n");

    VariableTable *table = create_variable_table(1); // Small table to test collisions

    // Test NULL checks
    assert(get_current_version(NULL, "x") == -1);
    assert(get_current_version(table, NULL) == -1);
    assert(!has_variable(NULL, "x"));
    assert(!has_variable(table, NULL));

    // Test invalid version access
    assert(get_version_definition(table, "x", -1) == NULL);
    assert(get_version_definition(table, "x", 0) == NULL);

    // Test non-existent variable
    assert(get_current_definition(table, "nonexistent") == NULL);
    assert(get_version_count(table, "nonexistent") == 0);

    // Create a version and test boundary conditions
    ZIRValue *val = create_mock_zir_value(1);
    create_new_version(table, "x", val);

    assert(get_version_definition(table, "x", -1) == NULL);
    assert(get_version_definition(table, "x", 1) == NULL);

    // Clean up
    free_mock_zir_value(val);
    destroy_variable_table(table);

    printf("✓ Edge cases test passed\n");
}

// Test hash table resizing behavior
void test_hash_collisions(void)
{
    printf("\nTesting hash collisions...\n");

    // Create a very small table to force collisions
    VariableTable *table = create_variable_table(1);

    // Create several variables (they should all hash to the same bucket)
    ZIRValue *vals[5];
    char var_names[5][10] = {"var1", "var2", "var3", "var4", "var5"};

    for (int i = 0; i < 5; i++)
    {
        vals[i] = create_mock_zir_value(i + 1);
        create_new_version(table, var_names[i], vals[i]);
    }

    // Verify all variables are accessible
    for (int i = 0; i < 5; i++)
    {
        assert(has_variable(table, var_names[i]));
        assert(get_current_definition(table, var_names[i]) == vals[i]);
    }

    // Clean up
    for (int i = 0; i < 5; i++)
    {
        free_mock_zir_value(vals[i]);
    }
    destroy_variable_table(table);

    printf("✓ Hash collisions test passed\n");
}

// Test array resizing
void test_array_resizing(void)
{
    printf("\nTesting definition array resizing...\n");

    VariableTable *table = create_variable_table(10);
    ZIRValue **vals = malloc(10 * sizeof(ZIRValue *));

    // Create more versions than the initial capacity
    for (int i = 0; i < 10; i++)
    {
        vals[i] = create_mock_zir_value(i + 1);
        int version = create_new_version(table, "x", vals[i]);
        assert(version == i);
        assert(get_current_definition(table, "x") == vals[i]);
    }

    // Verify all versions are accessible
    for (int i = 0; i < 10; i++)
    {
        assert(get_version_definition(table, "x", i) == vals[i]);
    }

    // Clean up
    for (int i = 0; i < 10; i++)
    {
        free_mock_zir_value(vals[i]);
    }
    free(vals);
    destroy_variable_table(table);

    printf("✓ Array resizing test passed\n");
}

int main(void)
{
    printf("Running variable tracking tests...\n");

    test_table_creation();
    test_version_tracking();
    test_multiple_variables();
    test_edge_cases();
    test_hash_collisions();
    test_array_resizing();

    printf("\nAll variable tracking tests passed! ✓\n");
    return 0;
}